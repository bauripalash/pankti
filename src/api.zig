const runCode = @import("run.zig").runCode;
export fn runCodeApi(source : [*]u8 , len : u32) bool {
    return runCode(source[0..@intCast(len)]);
}
