const std = @import("std");
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    var srcCore = [_][]const u8{
        "src/ast.c",
        "src/bengali.c",
        "src/core.c",
        "src/env.c",
        "src/interpreter.c",
        "src/lexer.c",
        "src/native.c",
        "src/object.c",
        "src/parser.c",
        "src/strescape.c",
        "src/token.c",
        "src/ustring.c",
        "src/utils.c",

        // Stdlib
        "src/stdlib/pstdlib.c",
        "src/stdlib/stdmap.c",
        "src/stdlib//stdmath.c",
        "src/stdlib/stdos.c",
        "src/stdlib/stdstring.c",

        // GC Related
        "src/gc/gc.c",
        "src/gc/gcexpr.c",
        "src/gc/gcobject.c",
        "src/gc/gcstmt.c",
    };

    var srcExternal = [_][]const u8{
        "src/external/stb/stb_ds_impl.c",
        "src/external/xxhash/xxhash.c",
    };

    var srcMain = [_][]const u8{
        "src/main.c",
    };

    var testsMain = [_][]const u8{
        "tests/test_main.c",
    };

    var testsSrcs = [_][]const u8{
        "tests/test_lexer.c",
    };

    var debugFlags = [_][]const u8{
        "-Wall",
        "-std=c11",
        "-g3",
    };

    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    mod.addCSourceFiles(.{ .files = &srcCore, .flags = &debugFlags });
    mod.addCSourceFiles(.{ .files = &srcMain, .flags = &debugFlags });
    mod.addCSourceFiles(.{ .files = &srcExternal, .flags = &debugFlags });

    const testsMod = b.createModule(.{
        .root_source_file = b.path("tests/test_main.zig"),
        .target = target,
        .optimize = optimize,
    });

    testsMod.addCSourceFiles(.{ .files = &srcCore, .flags = &debugFlags });
    testsMod.addCSourceFiles(.{ .files = &srcExternal, .flags = &debugFlags });
    testsMod.addCSourceFiles(.{ .files = &testsMain, .flags = &debugFlags });
    testsMod.addCSourceFiles(.{ .files = &testsSrcs, .flags = &debugFlags });

    const exe = b.addExecutable(.{
        .name = "pankti",
        .root_module = mod,
    });

    const testsExe = b.addExecutable(.{
        .name = "pankti_tests",
        .root_module = testsMod,
    });

    exe.linkLibC();
    testsExe.linkLibC();

    b.installArtifact(exe);

    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const tests_build_step = b.step("ntests", "Build Native Tests");
    const tests_build_cmd = b.addInstallArtifact(testsExe, .{});
    tests_build_step.dependOn(&tests_build_cmd.step);

    const tests_run_step = b.step("runtests", "Run Tests");
    const tests_run_cmd = b.addRunArtifact(testsExe);
    tests_run_step.dependOn(&tests_run_cmd.step);

    if (b.args) |args| {
        tests_run_cmd.addArgs(args);
    }
}
