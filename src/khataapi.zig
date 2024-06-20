const std = @import("std");
const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const Vm = @import("vm.zig").Vm;

pub export fn freeCode(src: [*c]u8, len: u32) void {
    // Does this even work ?
    std.heap.c_allocator.free(src[0..len]);
}

pub export fn runCode(src: [*]const u8, len: u32) callconv(.C) [*c]u8 {
    const handyAl = std.heap.c_allocator;
    const gcAl = std.heap.c_allocator;
    var result = std.fmt.allocPrint(handyAl, "error", .{}) catch return null;

    const rawSrc = src[0..len];

    var gc = Gc.new(gcAl, handyAl) catch {
        return result.ptr;
    };

    var warr = std.ArrayList(u8).init(gc.hal());

    gc.boot(warr.writer().any(), warr.writer().any());

    const source = utils.u8tou32(rawSrc, gc.hal()) catch {
        return result.ptr;
    };

    utils.printu32(source, std.io.getStdErr().writer().any());

    var myVm = Vm.newVm(gc.hal()) catch {
        return result.ptr;
    };

    myVm.bootVm(gc);

    const vmResult = myVm.interpret(source);

    myVm.freeVm(gc.hal());

    gc.hal().free(source);

    switch (vmResult) {
        .Ok => {
            handyAl.free(result);
            result = std.fmt.allocPrintZ(handyAl, "{s}", .{warr.items}) catch return null;
            return result.ptr;
        },
        .RuntimeError => return result.ptr,
        .CompileError => return result.ptr,
    }
}
