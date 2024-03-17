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

pub const Name = &[_]u32{ 'f', 'i', 'l', 'e' };
pub const NameFuncRead = &[_]u32{ 'r', 'e', 'a', 'd' };

pub fn file_Read(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "read(path) requires a single function").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "read(path) first argument must be a string").?;
    }

    if (!values[0].asObj().isString()) {
        return PValue.makeError(vm.gc, "read(path) first argument must be a string").?;
    }

    const rawFilePath = values[0].toString(vm.gc.hal()) catch return vm.gc.makeString("");

    vm.gc.hal().free(rawFilePath);
    std.debug.print("-> {s}\n", .{rawFilePath});

    //    const f = std.fs.cwd().readFileAlloc(vm.gc.hal(), rawFilePath, 10240);
    //  _ = f;

    std.debug.print("{any}", .{values[0].asObj().asString().chars});

    return PValue.makeNil();
}
