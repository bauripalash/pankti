//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("value.zig");
const Vm = @import("vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("utils.zig");
const valueerrors = @import("value_errors.zig");
const CopyError = valueerrors.CopyError;

pub fn nCopy(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "copy(...) takes a single argument").?;
    }

    const a = values[0];
    const result = a.createCopy(vm.gc) catch |e| {
        return PValue.makeComptimeError(
            vm.gc,
            "copy(...) failed due to internal error : {s}",
            .{valueerrors.copyErrorToString(e)},
        ).?;
    };

    return result;
}
pub fn nClock(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = vm;
    _ = values;
    _ = argc;
    if (utils.IS_WASM) {
        return PValue.makeNumber(@floatFromInt(utils.getTimestamp()));
    } else {
        const s = std.time.timestamp();
        return PValue.makeNumber(@floatFromInt(s));
    }
}

pub fn nShow(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc == 1) {
        const first = values[0];
        if (first.isObj() and first.asObj().isString()) {
            if (!first.asObj().asString().printWithoutQuotes(vm.gc)) {
                return PValue.makeNil();
            }
            vm.gc.pstdout.print(
                "\n",
                .{},
            ) catch return PValue.makeNil(); //Until complete

            return PValue.makeNil();
        }
    }
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        _ = values[i].printVal(vm.gc);
        vm.gc.pstdout.print(
            "\n",
            .{},
        ) catch return PValue.makeNil(); //Until complete
    }
    return PValue.makeNil();
}

pub fn nBnShow(vm: *Vm, argc: u8, values: []PValue) PValue { //WILL BE CHANGED?
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        _ = values[i].printVal(vm.gc);
        vm.gc.pstdout.print(
            "\n",
            .{},
        ) catch return PValue.makeNil(); //Until complete
    }
    return PValue.makeNil();
}

pub fn nLen(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "len(..) function only takes single argument",
        ).?;
    }

    if (values[0].getLen()) |len| {
        return PValue.makeNumber(@floatFromInt(len));
    }

    return PValue.makeNil();
}
