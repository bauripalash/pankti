//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "neopank",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const apilib = b.addStaticLibrary(.{
        .name = "neopankapi",
        .root_source_file = .{ .path = "src/api.zig" },
        .target = target,
        .optimize = optimize,
    });

    b.installArtifact(exe);
    
    const buildApi = b.addInstallArtifact(apilib);
    buildApi.step.dependOn(b.getInstallStep());

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

    const output_exe = "zig-out/bin/neopank";
    const source_to_run = "sample/c.txt";
    const run_interp_step = b.addSystemCommand(&[_][]const u8{
        output_exe,
        source_to_run,
    });

    const run_interp = b.step("code", "Run source code sample");
    run_interp.dependOn(&run_interp_step.step);

    const run_unit_tests = b.addRunArtifact(unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);

    const make_api_lib = b.step("api", "Build api library");
    make_api_lib.dependOn(&buildApi.step);
}
