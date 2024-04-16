//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const builtin = @import("builtin");

const pankti_version = std.SemanticVersion{
    .major = 0,
    .minor = 4,
    .patch = 0,
};

const min_zig_version = "0.12.0-dev.3659+1e5075f81";

fn getVersion(b: *Build) []const u8 {
    const version_string = b.fmt("{d}.{d}.{d}", .{
        pankti_version.major,
        pankti_version.minor,
        pankti_version.patch,
    });
    const build_root_path = b.build_root.path orelse ".";
    var code: u8 = undefined;
    const git_info = b.runAllowFail(&[_][]const u8{
        "git", "-C", build_root_path, "describe", "--match", "*.*.*", "--tags",
    }, &code, .Ignore) catch {
        return version_string;
    };

    const git_desc = std.mem.trim(u8, git_info, " \n\r");

    switch (std.mem.count(u8, git_desc, "-")) {
        2 => {
            var it = std.mem.splitScalar(u8, git_desc, '-');
            _ = it.first();
            const commit_height = it.next().?;
            const commit_id = it.next().?;

            const new_version_string = b.fmt("{s}-dev.{s}+{s}", .{
                version_string,
                commit_height,
                commit_id[1..],
            });
            return new_version_string;
        },

        else => {
            return version_string;
        },
    }
}

pub fn build(b: *Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const webTarget = b.resolveTargetQuery(.{
        .cpu_arch = .wasm32,
        .os_tag = .freestanding,
    });

    const version_result = getVersion(b);
    const build_options = b.addOptions();
    const build_options_module = build_options.createModule();
    build_options.addOption([]const u8, "version_string", version_result);
    build_options.addOption(std.SemanticVersion, "version", try std.SemanticVersion.parse(version_result));

    //Standard Executable
    const exe = b.addExecutable(.{
        .name = "pankti",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    //Standard Static Library for Pankti
    const apilib = b.addStaticLibrary(.{
        .name = "neopankapi",
        .root_source_file = .{ .path = "src/lekhokapi.zig" },
        .target = target,
        .optimize = .ReleaseFast,
    });

    apilib.linkLibC();

    const webExe = b.addExecutable(.{
        .name = "pankti",
        .root_source_file = .{ .path = "src/api.zig" },
        .target = webTarget,
        .optimize = .ReleaseSmall,
    });

    webExe.entry = .disabled;
    webExe.rdynamic = true;
    //webExe.import_memory = true;
    webExe.stack_size = std.wasm.page_size;

    const buildApi = b.addInstallArtifact(apilib, .{});
    _ = apilib.getEmittedH();
    buildApi.step.dependOn(b.getInstallStep());

    if (target.result.os.tag == .windows) {}

    const zig_libui_ng = b.dependency("zig_libui_ng", .{
        .target = target,
        .optimize = optimize,
    });

    const ideexe = b.addExecutable(.{
        .name = "panktilekhok",
        .root_source_file = .{ .path = "src/ide.zig" },
        .target = target,
        .optimize = optimize,
    });

    exe.root_module.addImport("build_options", build_options_module);

    ideexe.root_module.addImport("ui", zig_libui_ng.module("ui"));
    ideexe.root_module.addImport("build_options", build_options_module);
    //exe.linkLibrary(zig_libui_ng.artifact("ui"));

    //b.installArtifact(exe);

    const ide_step = b.step("ide", "Run the app");
    const ideExeInstall = b.addInstallArtifact(ideexe, .{});
    ide_step.dependOn(&ideExeInstall.step);

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const unit_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    if (target.result.os.tag == .windows) {
        exe.linkLibC();
        //exe.addObjectFile(std.Build.LazyPath.relative("winres/pankti.res.obj"));
        exe.addWin32ResourceFile(.{
            .file = .{ .path = "windows/windows.rc" },
        });
        unit_tests.linkLibC();
        ideexe.subsystem = .Windows;
        ideexe.addWin32ResourceFile(.{
            .file = .{ .path = "windows/panktikhata/panktikhata.rc" },
            //.flags = &.{ "/d", "_UI_STATIC" },
        });
        ideexe.linkLibC();
    }

    const run_unit_tests = b.addRunArtifact(unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);

    const make_api_lib = b.step("api", "Build api library");
    make_api_lib.dependOn(&buildApi.step);

    const wasmBuildStep = b.step("wasm", "Build Wasm");
    //wasmBuildStep.dependOn(&webExe.step);
    const webExeInstall = b.addInstallArtifact(webExe, .{});
    wasmBuildStep.dependOn(&webExeInstall.step);
}

const Build = blk: {
    const min_zig = std.SemanticVersion.parse(min_zig_version) catch unreachable;

    if (builtin.zig_version.order(min_zig) == .lt) {
        const msg = std.fmt.comptimePrint(
            \\Pankti Programming Language interpreter (codename: neopank)
            \\
            \\[BUILD FAILED]
            \\
            \\Required Zig version is greater than the present zig version;
            \\Minimum Required Zig Version : {s}
            \\         Current Zig Version : {any}
        , .{ .min_req = min_zig_version, .got = builtin.zig_version });

        @compileError(msg);
    }
    break :blk std.Build;
};
