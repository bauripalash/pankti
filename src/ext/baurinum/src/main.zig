const std = @import("std");
const bnum = @import("bnum.zig");

pub fn main() !void {
    var ga = std.heap.GeneralPurposeAllocator(.{}){};
    const al = ga.allocator();

    defer {
        _ = ga.deinit();
    }

    const x = try bnum.Bnum.new(al);
    const y = try bnum.Bnum.new(al);

    //_ = x.setstr(al, "-123");
    _ = x.seti64(al, 9999);
    _ = y.seti64(al, 100);

    const z = y.sub(x, al) orelse return;
    z.print(true);
    const p = z.toString(al, true) orelse return;
    std.debug.print("{s} -> {d}\n", .{ p, z.toI64() });

    al.free(p);
    //std.debug.print("->{any}\n", .{x.comp(y)});
    //x.print(true);
    x.free(al);
    y.free(al);
    z.free(al);
}
