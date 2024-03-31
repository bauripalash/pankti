const std = @import("std");
const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const Vm = @import("vm.zig").Vm;

export fn runCodeApi(rawrawSource: [*]const u8, len: u32) [*c]u8 {
    const handyAl = std.heap.c_allocator;
    const gcAl = std.heap.c_allocator;
    const rawSource = rawrawSource[0..len];

    var gc = Gc.new(gcAl, handyAl) catch {
        return null;
    };

    var w = std.ArrayList(u8).init(gc.hal());

    gc.boot(w.writer().any(), w.writer().any());
    const source = utils.u8tou32(rawSource, gc.hal()) catch {
        return null;
    };

    //utils.printu32(source, std.io.getStdOut().writer().any());

    var myVm = Vm.newVm(gc.hal()) catch {
        return null;
    };

    myVm.bootVm(gc);

    const result = myVm.interpret(source);

    myVm.freeVm(gc.hal());
    gc.hal().free(source);

    //std.debug.print("[{d}]", .{w.items.len});
    //std.debug.print("<{d}>", .{w.items});
    std.debug.print("\n<{s}>\n", .{w.items.ptr[@intFromPtr(w.items.ptr) .. @intFromPtr(w.items.ptr) + len]});
    switch (result) {
        .Ok => return w.items.ptr,
        .RuntimeError => return null,
        .CompileError => return null,
    }
}
