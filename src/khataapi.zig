const std = @import("std");
const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const Vm = @import("vm.zig").Vm;

pub export fn freeCode(src: [*]u8) void {
    std.heap.c_allocator.free(src);
}

pub export fn runCode(src: [*]const u8, len: u32) callconv(.C) ?[*]u8 {
    const handyAl = std.heap.c_allocator;
    const gcAl = std.heap.c_allocator;

    const rawSrc = src[0..len];

    var gc = Gc.new(gcAl, handyAl) catch {
        return null;
    };

    var warr = std.ArrayList(u8).init(gc.hal());

    gc.boot(warr.writer().any(), warr.writer().any());

    const source = utils.u8tou32(rawSrc, gc.hal()) catch {
        return null;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        return null;
    };

    myVm.bootVm(gc);

    const vmResult = myVm.interpret(source);

    myVm.freeVm(gc.hal());

    gc.hal().free(source);

    switch (vmResult) {
        .Ok => {
            const result = std.fmt.allocPrint(handyAl, "{s}", warr.items.ptr) catch return null;

            return result.ptr;
        },
        .RuntimeError => return null,
        .CompileError => return null,
    }
}
