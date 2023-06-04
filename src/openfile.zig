const std = @import("std");
const print = std.debug.print;

pub fn openfile(path: []const u8, aloc: std.mem.Allocator) ![]u8 {
    var f = try std.fs.cwd().openFile(path, .{});
    defer f.close();

    const read_buf = try f.readToEndAlloc(aloc, 1024);
    return read_buf;
}
