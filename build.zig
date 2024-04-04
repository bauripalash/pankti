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

const min_zig_version = "0.12.0-dev.3496+a2df84d0f";

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const webTarget = b.resolveTargetQuery(.{
        .cpu_arch = .wasm32,
        .os_tag = .freestanding,
    });

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

    if (target.result.os.tag == .windows) {
        exe.linkLibC();
        exe.addObjectFile(std.Build.LazyPath.relative("winres/pankti.res.obj"));
    }

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

    ideexe.root_module.addImport("ui", zig_libui_ng.module("ui"));
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
        unit_tests.linkLibC();
        ideexe.subsystem = .Windows;
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
