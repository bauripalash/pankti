const std = @import("std");

const VERSION_FILE = "PANKTI_VERSION.txt";

const GitInfo = struct {
    hash: []const u8,
    count: []const u8,
};

pub fn getGitInfo(b: *std.Build) GitInfo {
    var result: GitInfo = .{
        .count = "",
        .hash = "",
    };

    var code: u8 = undefined;

    const hashResult = b.runAllowFail(
        &[_][]const u8{ "git", "rev-parse", "--short", "HEAD" },
        &code,
        .Ignore,
    ) catch "";

    var tagName = b.runAllowFail(
        &[_][]const u8{ "git", "describe", "--tags", "--abbrev=0" },
        &code,
        .Ignore,
    ) catch "";

    var countResult: []u8 = undefined;
    if (code != 0) {
        countResult = b.runAllowFail(
            &[_][]const u8{ "git", "rev-list", "--count", "HEAD" },
            &code,
            .Ignore,
        ) catch "";
    } else {
        tagName = std.mem.trim(u8, tagName, "\n\r \t");
        countResult = b.runAllowFail(
            &[_][]const u8{
                "git",
                "rev-list",
                "--count",
                b.fmt("{s}..HEAD", .{tagName}),
            },
            &code,
            .Ignore,
        ) catch "";
    }

    result.hash = std.mem.trim(u8, hashResult, "\n\r \t");
    result.count = std.mem.trim(u8, countResult, "\n\r \t");

    return result;
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const gitinfo = getGitInfo(b);

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
        "src/printer.c",
        "src/strescape.c",
        "src/token.c",
        "src/ustring.c",
        "src/unicode.c",
        "src/utils.c",

        // Stdlib
        "src/stdlib/pstdlib.c",
        "src/stdlib/stdmap.c",
        "src/stdlib/stdarray.c",
        "src/stdlib/stdmath.c",
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
        "src/external/gb/gb_string_impl.c",
        "src/external/xxhash/xxhash.c",

        //libgrapheme
        "src/external/libgrapheme/src/case.c",
        "src/external/libgrapheme/src/character.c",
        "src/external/libgrapheme/src/line.c",
        "src/external/libgrapheme/src/sentence.c",
        "src/external/libgrapheme/src/utf8.c",
        "src/external/libgrapheme/src/util.c",
        "src/external/libgrapheme/src/word.c",
    };

    var srcMain = [_][]const u8{
        "src/main.c",
    };

    var frontendTestMain = [_][]const u8{
        "tests/frontend/test_main.c",
    };

    var frontendTestSrcs = [_][]const u8{
        "tests/frontend/test_lexer.c",
        "tests/frontend/test_parser.c",
    };

    var runtimeTestMain = [_][]const u8{"tests/runtime/test_runtime.c"};

    var debugFlags = [_][]const u8{ "-Wall", "-std=c11", "-g3" };

    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    mod.addCSourceFiles(.{ .files = &srcCore, .flags = &debugFlags });
    mod.addCSourceFiles(.{ .files = &srcMain, .flags = &debugFlags });
    mod.addCSourceFiles(.{ .files = &srcExternal, .flags = &debugFlags });

    const frontendTestsMod = b.createModule(.{
        .root_source_file = b.path("tests/frontend/test_main.zig"),
        .target = target,
        .optimize = optimize,
    });

    frontendTestsMod.addCSourceFiles(.{ .files = &srcCore, .flags = &debugFlags });
    frontendTestsMod.addCSourceFiles(.{ .files = &srcExternal, .flags = &debugFlags });
    frontendTestsMod.addCSourceFiles(.{ .files = &frontendTestMain, .flags = &debugFlags });
    frontendTestsMod.addCSourceFiles(.{ .files = &frontendTestSrcs, .flags = &debugFlags });

    const runtimeTestsMod = b.createModule(.{
        .root_source_file = b.path("tests/runtime/test_runtime.zig"),
        .target = target,
        .optimize = optimize,
    });

    runtimeTestsMod.addCSourceFiles(.{ .files = &runtimeTestMain, .flags = &debugFlags });

    const exe = b.addExecutable(.{
        .name = "pankti",
        .root_module = mod,
    });

    const frontendTestExe = b.addExecutable(.{
        .name = "pankti_tests",
        .root_module = frontendTestsMod,
    });
    const runtimeTestsExe = b.addExecutable(.{
        .name = "pankti_tests",
        .root_module = runtimeTestsMod,
    });

    exe.linkLibC();
    frontendTestExe.linkLibC();
    runtimeTestsExe.linkLibC();

    mod.addCMacro("PANKTI_GIT_HASH", b.fmt("\"{s}\"", .{gitinfo.hash}));
    frontendTestsMod.addCMacro("PANKTI_GIT_HASH", b.fmt("\"{s}\"", .{gitinfo.hash}));
    runtimeTestsMod.addCMacro("PANKTI_GIT_HASH", b.fmt("\"{s}\"", .{gitinfo.hash}));

    mod.addCMacro("PANKTI_GIT_COUNT", b.fmt("\"{s}\"", .{gitinfo.count}));
    frontendTestsMod.addCMacro("PANKTI_GIT_COUNT", b.fmt("\"{s}\"", .{gitinfo.count}));
    runtimeTestsMod.addCMacro("PANKTI_GIT_COUNT", b.fmt("\"{s}\"", .{gitinfo.count}));

    mod.addCMacro("USE_NAN_BOXING", "1");
    frontendTestsMod.addCMacro("USE_NAN_BOXING", "1");
    runtimeTestsMod.addCMacro("USE_NAN_BOXING", "1");

    mod.addCMacro("USING_ZIG_BUILD", "1");
    frontendTestsMod.addCMacro("USING_ZIG_BUILD", "1");
    runtimeTestsMod.addCMacro("USING_ZIG_BUILD", "1");

    if (target.result.os.tag == .linux) {
        mod.addCMacro("PANKTI_OS_LINUX", "1");
        frontendTestsMod.addCMacro("PANKTI_OS_LINUX", "1");
        runtimeTestsMod.addCMacro("PANKTI_OS_LINUX", "1");
    } else if (target.result.os.tag == .windows) {
        mod.addCMacro("PANKTI_OS_WIN", "1");
        frontendTestsMod.addCMacro("PANKTI_OS_WIN", "1");
        runtimeTestsMod.addCMacro("PANKTI_OS_WIN", "1");
    } else if (target.result.os.tag == .macos) {
        mod.addCMacro("PANKTI_OS_MAC", "1");
        frontendTestsMod.addCMacro("PANKTI_OS_MAC", "1");
        runtimeTestsMod.addCMacro("PANKTI_OS_MAC", "1");
    } // Web platform not yet available

    if (optimize == .Debug) {
        mod.addCMacro("PANKTI_BUILD_DEBUG", "1");
        frontendTestsMod.addCMacro("PANKTI_BUILD_DEBUG", "1");
        runtimeTestsMod.addCMacro("PANKTI_BUILD_DEBUG", "1");
    } else {
        mod.addCMacro("PANKTI_BUILD_RELEASE", "1");
        frontendTestsMod.addCMacro("PANKTI_BUILD_RELEASE", "1");
        runtimeTestsMod.addCMacro("PANKTI_BUILD_RELEASE", "1");
    }

    //frontendTestsMod.addCMacro("PANKTI_VERSION", b.fmt("\"{s}\"", .{versionInfo}));

    b.installArtifact(exe);

    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const tests_build_step = b.step("ntests", "Build Native Tests");
    const tests_build_cmd = b.addInstallArtifact(frontendTestExe, .{});
    tests_build_step.dependOn(&tests_build_cmd.step);

    const tests_run_step = b.step("runtests", "Run Tests");
    const tests_run_cmd = b.addRunArtifact(frontendTestExe);
    const tests_runtime_run_cmd = b.addRunArtifact(runtimeTestsExe);

    tests_run_step.dependOn(&tests_run_cmd.step);
    tests_run_step.dependOn(&tests_runtime_run_cmd.step);

    if (b.args) |args| {
        tests_run_cmd.addArgs(args);
    }
}
