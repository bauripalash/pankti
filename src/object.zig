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
const Gc = @import("gc.zig").Gc;

pub const PObj = struct {
    objtype : OType,
    next : ?*PObj,
    isMarked : bool,
    
    pub const OType = enum(u8) {
        Ot_String,

        pub fn toString(self : OType) []const u8 {

            switch (self) {
                .Ot_String => { return "OBJ_STRING"; }
            }
        }
    };

    pub fn create(gc : *Gc , comptime T : type , objtype : OType) !*PObj{
        const ptr = try gc.getAlc().create(T);
        ptr.obj = PObj{
            .next = gc.objects,
            .objtype = objtype,
            .isMarked = false,
        };

        gc.objects = &ptr.obj;

        return &ptr.obj;
    }

    pub fn child(self : *PObj , comptime ChildType : type) *ChildType{
        return @fieldParentPtr(ChildType, "obj", self);
    }

    pub fn getType(self : *PObj) OType {
        return self.objtype;
    }

    pub fn is(self : *PObj , tp : OType) bool{
        return self.getType() == tp;
    }

    pub fn isString(self : *PObj) bool {
        return self.is(.Ot_String);
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
            .Ot_String => self.asString().print(),
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
        hash : u32,
        len : usize,

        fn allocate(gc : *Gc , chars : []const u32) !*OString{
            const obj = try PObj.create(gc, PObj.OString, .Ot_String);
            const str = obj.asString();
            var temp_chars = try gc.getAlc().alloc(u32, chars.len);
            str.len = chars.len;
            str.hash = utils.hashU32(chars) catch 0;
            @memcpy(temp_chars.ptr, chars);
            str.chars = temp_chars;


            return str;
            
        }

        pub fn free(self : *OString , gc : *Gc) void{
            gc.getAlc().free(self.chars);
            gc.getAlc().destroy(self);
        }

        pub fn print(self : *OString) void{
            std.debug.print("\"" , .{});
            utils.printu32(self.chars);
            std.debug.print("\"" , .{});
        }

        pub fn size(self : *OString) usize{
            var total : usize = @sizeOf(OString);
            total += self.chars.len + 1 * @sizeOf(u32); 
            //+1 is for self.hash each a u32

            total += @sizeOf(@TypeOf(self.len));
            return total;
            
        }

        pub fn parent(self : *OString) *PObj{
            return @ptrCast(*PObj, self);
        }
    };

};



