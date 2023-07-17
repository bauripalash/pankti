const std = @import("std");
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const PObj = @import("../object.zig").PObj;

pub const Name = &[_]u32{ 'b', 'i', 'g' };

pub const NameFuncNew = &[_]u32{ 'i', 'n', 't' };
pub fn big_New(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 1) {
        return PValue.makeError(vm.gc, "new(value) function only takes single argument").?;
    }

    const bigobj = PObj.OBigNum.newInt(vm) orelse {
        return PValue.makeError(vm.gc, "failed to create big number").?;
    };

    vm.stack.push(PValue.makeObj(bigobj.parent())) catch return PValue.makeNil();

    _ = bigobj.ival.?.setu64(vm.gc.hal(), 100);

    return vm.stack.pop() catch return PValue.makeNil();
}
