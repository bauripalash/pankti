const std = @import("std");
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const PObj = @import("../object.zig").PObj;

pub const Name = &[_]u32{ 'b', 'i', 'g' };

pub const NamefuncSub = &[_]u32{ 's', 'u', 'b' };
pub fn big_Sub(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "sub(a,b) function only takes 2 arguments",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "sub(a,b) takes only big numbers",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "sub(a,b) takes only big numbers",
        ).?;
    }

    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const resultInt = aInt.ival.sub(bInt.ival, vm.gc.hal()) orelse return PValue.makeNil();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    x.ival = resultInt;

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncAdd = &[_]u32{ 'a', 'd', 'd' };
pub fn big_Add(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "add(a,b) function only takes 2 arguments",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "add(a,b) takes only big numbers",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "add(a,b) takes only big numbers",
        ).?;
    }

    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const resultInt = aInt.ival.add(bInt.ival, vm.gc.hal()) orelse return PValue.makeNil();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    x.ival = resultInt;

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NameFuncNew = &[_]u32{ 'n', 'e', 'w' };
pub fn big_New(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "new(value) function only takes single argument",
        ).?;
    }

    const item = values[0];

    if (!item.isString() and !item.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "new(value) takes only string or number",
        ).?;
    }

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    _ = x.initInt(vm.gc);

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    if (item.isString()) {
        const rawString = item.asObj().asString();

        const u8string = utils.u32tou8(rawString.chars, vm.gc.hal()) catch {
            return PValue.makeNil();
        };

        if (!x.ival.setstr(vm.gc.hal(), u8string)) {
            return PValue.makeNil();
        }

        vm.gc.hal().free(u8string);
    } else if (item.isNumber()) {
        const number = item.asNumber();

        if (!x.ival.seti64(vm.gc.hal(), @intFromFloat(number))) {
            return PValue.makeNil();
        }
    }

    return vm.stack.pop() catch return PValue.makeNil();
}
