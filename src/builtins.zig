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

    const links = vm.gc.hal().create(value.ParentLink) catch return PValue.makeNil();
    links.prev = std.ArrayListUnmanaged(PValue){};

    const a = values[0];
    const result = a.createCopy(vm.gc, links) catch |e| {
        _ = links.free(vm.gc);
        return PValue.makeComptimeError(
            vm.gc,
            "copy(...) failed due to internal error : {s}",
            .{valueerrors.copyErrorToString(e)},
        ).?;
    };

    _ = links.free(vm.gc);

    return result;
}

pub fn nShow(vm: *Vm, argc: u8, values: []PValue) PValue {
    const links = vm.gc.hal().create(
        value.ParentLink,
    ) catch return PValue.makeNil();

    links.prev = std.ArrayListUnmanaged(PValue){};

    var i: usize = 0;
    while (i < argc) : (i += 1) {
        _ = values[i].printValForShow(vm.gc, links);
    }

    _ = links.free(vm.gc);

    return PValue.makeNil();
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
