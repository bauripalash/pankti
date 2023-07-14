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

const MapTableContext = struct {
    pub fn eql(self: @This(), a: PValue, b: PValue) bool {
        _ = self;

        return a.hash() == b.hash();
    }

    pub fn hash(self: @This(), key: PValue) u64 {
        _ = self;
        return @as(u64, @intCast(key.hash()));
    }
};

pub fn MapTable() type {
    return std.HashMapUnmanaged(PValue, PValue, MapTableContext, std.hash_map.default_max_load_percentage);
}

const PankTableContext = struct {
    pub fn eql(
        self: @This(),
        a: *Pobj.OString,
        b: *Pobj.OString,
    ) bool {
        _ = self;
        return a.hash == b.hash;
    }
    pub fn hash(self: @This(), key: *Pobj.OString) u64 {
        _ = self;
        return @as(u64, @intCast(key.hash));
    }
};

pub fn PankTable() type {
    return std.HashMapUnmanaged(*Pobj.OString, PValue, PankTableContext, std.hash_map.default_max_load_percentage);
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
