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
const msl = stdlib.msl;
const builtin = @import("builtin");

pub const Name = &[_]u32{ 's', 't', 'r', 'i', 'n', 'g' };
pub const NameFuncSplit = &[_]u32{ 's', 'p', 'l', 'i', 't' };
pub fn str_Split(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(vm.gc, "split(a, b) function only takes 2 arguments").?;
    }

    if (!values[0].isString() or !values[0].isString()) {
        return PValue.makeError(vm.gc, "for split(a ,b) both arguments must be string");
    }

    const rawString = values[0].asObj().asString();
    _ = rawString;

    const rawDelim = values[0].asObj().asString();

    if (rawDelim.chars.len != 1) {
        return PValue.makeError(vm.gc, "delimiter must be a single char string");
    }

    const delim = rawDelim.chars[0];
    _ = delim;
}
