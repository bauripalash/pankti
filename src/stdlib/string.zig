//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const PObj = @import("../object.zig").PObj;

pub const Name = &[_]u32{ 's', 't', 'r', 'i', 'n', 'g' };

pub const NameFuncString = &[_]u32{ 's', 't', 'r' };
pub fn str_String(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "str(value) function only takes single argument").?;
    }

    const rawString = values[0].toString(vm.gc.hal()) catch return vm.gc.makeString("");
    const myString = utils.u8tou32(rawString, vm.gc.hal()) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };
    const strObj = vm.gc.copyString(myString, @intCast(myString.len)) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };

    vm.stack.push(PValue.makeObj(strObj.parent())) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };

    vm.gc.hal().free(rawString);
    vm.gc.hal().free(myString);

    return vm.stack.pop() catch return vm.gc.makeString("");
}
pub const NameFuncSplit = &[_]u32{ 's', 'p', 'l', 'i', 't' };
pub fn str_Split(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(vm.gc, "split(a, b) function only takes 2 arguments").?;
    }

    if (!values[0].isString() or !values[0].isString()) {
        return PValue.makeError(vm.gc, "for split(a ,b) both arguments must be string").?;
    }

    const rawString = values[0].asObj().asString();

    const rawDelim = values[1].asObj().asString();

    var objArr = vm.gc.newObj(PObj.OType.Ot_Array, PObj.OArray) catch {
        return PValue.makeNil();
    };

    objArr.init();

    vm.stack.push(PValue.makeObj(objArr.parent())) catch return PValue.makeNil();

    var ite = std.mem.splitAny(u32, rawString.chars, rawDelim.chars);
    while (ite.next()) |s| {
        const objString = vm.gc.copyString(s, @intCast(s.len)) catch return PValue.makeNil();
        vm.stack.push(PValue.makeObj(objString.parent())) catch return PValue.makeNil();
        if (!objArr.addItem(vm.gc, vm.stack.pop() catch return PValue.makeNil())) {
            return PValue.makeNil();
        }
    }

    return vm.stack.pop() catch return PValue.makeNil();
}
