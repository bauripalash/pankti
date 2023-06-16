//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const Vm = @import("vm.zig").Vm;
const value = @import("value.zig");
const PValue = value.PValue;
const utils = @import("utils.zig");

pub const PObj = struct {
    objtype : OType,
    next : ?*PObj,
    isMarked : bool,
    
    pub const OType = enum(u8) {
        Ot_String,
    };

    pub fn create(vm : *Vm , comptime T : type , objtype : OType) !*PObj{
        const ptr = try vm.al.create(T);
        ptr.obj = PObj{
            .next = null,
            .objtype = objtype,
            .isMarked = false,
        };

        return &ptr.obj;
    }

    pub fn getType(self : *PObj) OType {
        return self.objtype;
    }

    pub fn is(self : *PObj , tp : OType) bool{
        return self.getType() == tp;
    }

    pub fn asString(self : *PObj) *OString{
        return @fieldParentPtr(OString, "obj", self);
    }

    pub fn free(self : *PObj , vm : *Vm) void{
        switch (self.objtype) {
            .Ot_String => self.asString().free(vm),
            //else => {},
        }

    }

    pub fn asValue(self : *PObj) PValue{
        return PValue.makeObj(self);
    }

    pub fn printObj(self : *PObj) void{
        switch (self.getType()) {
            .Ot_String => utils.printu32(self.asString().chars),
        }
    }

    pub fn toString(self : *PObj , al : std.mem.Allocator) ![]u8{
        switch (self.getType()) {
           .Ot_String => {
                return try utils.u32tou8(self.asString().chars, al);
           }, 
        }

        return try utils.u32tou8([_]u32{'_'}, al);
    }

    
    pub const OString = struct {
        obj : PObj,
        chars : []u32,
        len : usize,

        fn allocate(vm : *Vm , chars : []const u32) !*OString{
            const obj = try PObj.create(vm, PObj.OString, .Ot_String);
            const str = obj.asString();
            var temp_chars = try vm.al.alloc(u32, chars.len);
            str.len = chars.len;
            @memcpy(temp_chars.ptr, chars);
            str.chars = temp_chars;


            return str;
            
        }

        pub fn copy(vm : *Vm , chars : []const u32) !*OString{
            if (vm.strings.get(chars)) |s| {
                return s;
            }

            const s = try PObj.OString.allocate(vm, chars);

            //std.debug.print("{any}", .{s.*});
            try vm.strings.put(vm.al , chars , s);
            return s;
        }

        pub fn free(self : *OString , vm : *Vm) void{
            vm.al.free(self.chars);
            vm.al.destroy(self);
        }
    };

};



