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
