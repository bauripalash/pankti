//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const utils = @import("utils.zig");
const object = @import("object.zig");
const Vm = @import("vm.zig").Vm;
const Pobj = object.PObj;
const val = @import("value.zig");
const PValue = val.PValue;

const PankTableContext = struct {
    pub fn eql(
        self: @This(),
        a: *Pobj.OString,
        b: *Pobj.OString,
        index: usize,
    ) bool {
        _ = index;
        _ = self;
        return a.hash == b.hash;
    }
    pub fn hash(self: @This(), key: *Pobj.OString) u32 {
        _ = self;
        return key.hash;
    }
};

pub fn PankTable() type {
    return std.ArrayHashMapUnmanaged(
        *Pobj.OString,
        PValue,
        PankTableContext,
        true,
    );
}

pub fn freeGlobalsTable(vm: *Vm, table: PankTable()) bool {
    for (table.values()) |value| {
        value.free(vm);
    }

    return true;
}

pub fn getString(self: PankTable(), hash: u32, len: u32) ?*Pobj.OString {
    var ite = self.iterator();
    while (ite.next()) |n| {
        const rawStr: *Pobj.OString = n.key_ptr.*;

        if (rawStr.len == len and rawStr.hash == hash) {
            return rawStr;
        }
    }

    return null;
}
