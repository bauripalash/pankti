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

pub const Name = &[_]u32{ 'm', 'a', 'p' };

pub const NameFuncExists = &[_]u32{ 'e', 'x', 'i', 's', 't', 's' };
pub fn map_Exists(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(vm.gc, "exists(map , key) function only takes 2 arguments").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "exists(map , key) first argument must be a map").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "exists(map , key) first argument must be a map").?;
    }

    const rawMap = values[0].asObj().asHmap();

    if (rawMap.values.getKey(values[1])) |_| {
        return PValue.makeBool(true);
    } else {
        return PValue.makeBool(false);
    }
}

pub const NameFuncKeys = &[_]u32{ 'k', 'e', 'y', 's' };

pub fn map_Keys(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "keys(m) function only takes a single argument").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "keys(m) argument must be a map").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "keys(m) argument must be a map").?;
    }

    const rawMap = values[0].asObj().asHmap();

    var objArr = vm.gc.newObj(PObj.OType.Ot_Array, PObj.OArray) catch {
        return PValue.makeNil();
    };

    objArr.init();

    vm.stack.push(PValue.makeObj(objArr.parent())) catch {
        return PValue.makeNil();
    };

    var ite = rawMap.values.keyIterator();

    while (ite.next()) |key| {
        if (!objArr.addItem(vm.gc, key.*)) return PValue.makeNil();
    }

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NameFuncValues = &[_]u32{ 'v', 'a', 'l', 'u', 'e', 's' };

pub fn map_Values(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "values(m) function only takes a single argument").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "values(m) argument must be a map").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "values(m) argument must be a map").?;
    }

    const rawMap = values[0].asObj().asHmap();

    var objArr = vm.gc.newObj(PObj.OType.Ot_Array, PObj.OArray) catch {
        return PValue.makeNil();
    };

    objArr.init();

    vm.stack.push(PValue.makeObj(objArr.parent())) catch {
        return PValue.makeNil();
    };

    var ite = rawMap.values.valueIterator();

    while (ite.next()) |val| {
        if (!objArr.addItem(vm.gc, val.*)) return PValue.makeNil();
    }

    return vm.stack.pop() catch return PValue.makeNil();
}
