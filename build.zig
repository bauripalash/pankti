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
        "src/main.c",
        "src/native.c",
        "src/object.c",
        "src/parser.c",
        "src/token.c",
        "src/ustring.c",
        "src/utils.c",

        // GC Related
        "src/gc/gc.c",
        "src/gc/gcexpr.c",
        "src/gc/gcobject.c",
        "src/gc/gcstmt.c",
    };

    var srcExternal = [_][]const u8{
        "src/external/xxhash.c",
    };

    var debugFlags = [_][]const u8{
        "-Wall",
        "-std=gnu99",
        "-g3",
    };

    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    mod.addCSourceFiles(.{ .files = &srcCore, .flags = &debugFlags });
    mod.addCSourceFiles(.{ .files = &srcExternal, .flags = &debugFlags });

    const exe = b.addExecutable(.{
        .name = "pankti",
        .root_module = mod,
    });

    exe.linkLibC();

    b.installArtifact(exe);
    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
}
