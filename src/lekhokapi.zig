const std = @import("std");
const Gc = @import("gc.zig").Gc;
const _vm = @import("vm.zig");
const Vm = _vm.Vm;
const writer = @import("writer.zig");
const utils = @import("utils.zig");

extern fn writeStdout(ptr: usize, len: usize) void;

fn writeOutString(bts: []const u8) void {
    writeStdout(@intFromPtr(bts.ptr), bts.len);
}

export fn runCodeLekhok(rawSource: [*]const u8, len: u32) bool {
    const utf8Source = rawSource[0..len];

    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create a Garbage Collector\n", .{});
        return false;
    };

    const outWriter = std.io.getStdOut();

    gc.boot(outWriter.writer(), outWriter.writer());
    const source = utils.u8tou32(utf8Source, gc.hal()) catch {
        return false;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        return false;
    };

    myVm.bootVm(gc);

    const result = myVm.interpret(source);

    myVm.freeVm(gc.hal());
    gc.hal().free(source);

    switch (result) {
        .Ok => return true,
        .RuntimeError => return false,
        .CompileError => return false,
    }
}
