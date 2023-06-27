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

const StringTableContext = struct {
    pub fn eql(self : @This() , a : []const u32 , b : []const u32 , index : usize) bool{
        _ = index;
        _ = self;
        return std.mem.eql(u32, a, b);
    }

    pub fn hash(self : @This() , key : []const u32) u32{
        _ = self;
        
        const result = utils.hashU32(key) catch 0;
        //std.debug.print("<{any}->{d}>\n\n", .{key , result});
        return @intCast(result);
    }
};

pub fn StringTable() type{
    return std.ArrayHashMapUnmanaged([]const u32, *Pobj.OString, StringTableContext, true);
}

pub fn freeStringTable(vm : *Vm , table : StringTable()) bool {
    //std.debug.print("{d}" , .{table.keys().len});
    for (table.values()) |value| {
        value.free(vm);
    }
    return true;
}

const GlobalsTableContext = struct {
    
    pub fn eql(self : @This() , a :*Pobj.OString , b :*Pobj.OString , index : usize) bool {
        _ = index;
        _ = self;
        return a.hash == b.hash;
    }
    pub fn hash(self : @This() , key : *Pobj.OString) u32 {
        _ = self;
        return key.hash;
    }
};

pub fn GlobalsTable() type {
    return std.ArrayHashMapUnmanaged(*Pobj.OString, PValue, GlobalsTableContext, true);
}

pub fn freeGlobalsTable(vm : *Vm , table : GlobalsTable()) bool {

    for (table.values()) |value| {
        value.free(vm);
    }

    return true;

}
