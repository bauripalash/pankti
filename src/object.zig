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
const instruction = @import("instruction.zig");
const Instruction = instruction.Instruction;

pub const PObj = struct {
    objtype : OType,
    next : ?*PObj,
    isMarked : bool,
    
    pub const OType = enum(u8) {
        Ot_String,
        Ot_Function,
        Ot_NativeFunc,
        Ot_Closure,
        Ot_UpValue,

        pub fn toString(self : OType) []const u8 {

            switch (self) {
                .Ot_String => { return "OBJ_STRING"; },
                .Ot_Function => {return "OBJ_FUNC";},
                .Ot_NativeFunc => return "OBJ_NATIVE",
                .Ot_Closure => return "OBJ_CLOSURE",
                .Ot_UpValue => return "OBJ_UPVALUE",
                
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

    pub fn isEqual(self : *PObj , other : *PObj) bool {
        if (self.getType() != other.getType()) { return false; }

        if (self.isString()) {
            return self.asString().hash == other.asString().hash;
        } 

        return false;
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

    pub fn asFunc(self : *PObj) *OFunction{
        return self.child(PObj.OFunction);
    }

    pub fn asNativeFun(self : *PObj) *ONativeFunction {
        return self.child(PObj.ONativeFunction);
    }

    pub fn asClosure(self : *PObj) *OClosure {
        return self.child(PObj.OClosure);
    }
    
    pub fn asUpvalue(self : *PObj) *OUpValue {
        return self.child(PObj.OUpValue);
    }

    pub fn free(self : *PObj , vm : *Vm) void{
        switch (self.objtype) {
            .Ot_String => self.asString().free(vm),
            else => {},
        }

    }
    pub fn asValue(self : *PObj) PValue{
        return PValue.makeObj(self);
    }

    pub fn printObj(self : *PObj) void{
        switch (self.getType()) {
            .Ot_String => self.asString().print(),
            .Ot_Function => self.asFunc().print(),
            .Ot_NativeFunc => self.asNativeFun().print(),
            .Ot_Closure => self.asClosure().print(),
            .Ot_UpValue => self.asUpvalue().print(),
        }
    }

    pub fn toString(self : *PObj , al : std.mem.Allocator) ![]u8{
        switch (self.getType()) {
           .Ot_String => {
                return try utils.u32tou8(self.asString().chars, al);
           }, 

           .Ot_Function => {
                return try utils.u32tou8(&[_]u32{'<' , 'f' , 'n' , '>'}, al);
           },

            .Ot_Closure => {
                return try utils.u32tou8(&[_]u32{'c' , 'l' , 'o' , 's' , 'u' , 'r' , 'e'}, al);
            },

           .Ot_NativeFunc => {
    
                return try utils.u32tou8(&[_]u32{'<' ,
                    'n' ,
                    'a' ,
                    't',
                    'i',
                    'v',
                    ' ',
                    'f' ,
                    'n' ,
                    '>'}, al);
            },
            .Ot_UpValue => { return try utils.u32tou8(&[_]u32{ 'u' , 'p' , 'v' , 'a' , 'l' , 'u' ,'e'  } , al); },
        }

        return try utils.u32tou8([_]u32{'_'}, al);
    }

    pub const OUpValue = struct {
        obj : PObj,
        location : *PValue,
        next : ?*OUpValue, 
        closed : PValue,

        pub fn init(self : *OUpValue , val : *PValue) void{
            self.location = val;
            self.next = null;
            self.closed = PValue.makeNil();
        }

        pub fn print(self : *OUpValue) void {
            _ = self;
            std.debug.print("upvalue" , .{});
        }

        pub fn free(self : *OUpValue , gc : *Gc) void {
            gc.getAlc().destroy(self);
        }

        pub fn parent(self : *OUpValue) *PObj {
            return @ptrCast(self);
        }
    };

    pub const OClosure = struct {
        obj : PObj,
        function : *OFunction,
        upvalues : [*]*OUpValue,
        upc : u32,

        pub fn new(gc : *Gc , func : *OFunction) !*PObj.OClosure {
            const upvalues = try gc.getAlc().alloc(?*OUpValue, func.upvCount);
            for (upvalues) |*v| {
                v.* = std.mem.zeroes(?*OUpValue);
            }
            const cl = try gc.newObj(.Ot_Closure, PObj.OClosure);
            const ptr : [*]*OUpValue = @ptrCast(upvalues);
            cl.upvalues = ptr;
            cl.upc = func.upvCount;
            cl.function = func;
            return cl;
        }

        pub fn print(self : *OClosure) void {
            std.debug.print("<cl " , .{});
            self.function.print();
            std.debug.print(" >" , .{});
        }

        pub fn parent(self : *OClosure) *PObj {
            return @ptrCast(self);
        }

        pub fn free(self : *OClosure , gc : *Gc) void {
            gc.getAlc().free(self.upvalues[0..self.upc]);
            gc.getAlc().destroy(self);
        }
    };

    pub const ONativeFunction = struct {
        obj : PObj,
        func : NativeFn,

        pub const NativeFn = *const fn (u8 , []PValue) PValue;

        pub fn init(self : *ONativeFunction , func : NativeFn) void {
            self.func = func;
        }

        pub fn print(self : *ONativeFunction) void {
            _ = self;
            std.debug.print("<native fn>" , .{});
        }

        pub fn parent(self : *ONativeFunction) *PObj {
            return @ptrCast(self);
        }

        pub fn free(self : *ONativeFunction , gc : *Gc) void {
            gc.getAlc().destroy(self);
        } 
    };

    
    pub const OFunction = struct {
        obj : PObj,
        arity : u8,
        name : ?*OString,
        upvCount : u32,
        ins : Instruction,

        pub fn init(self : *OFunction , gc : *Gc) void {
            self.arity = 0;
            self.name = null;
            self.upvCount = 0;
            self.ins = Instruction.init(gc);
        }

        pub inline fn parent(self : *OFunction) *PObj {
            return @ptrCast(self);
        }

        pub fn getName(self : *OFunction) []const u32{
            if (self.name) |n| { 
                return n.chars[0..n.chars.len];
            } else {
                return &[_]u32 { '<', 's', 'c', 'r', 'i', 'p', 't', '>' };
            }
        }

        pub fn print(self : *OFunction) void {
            std.debug.print("<fn " , .{});
            utils.printu32(self.getName());
            std.debug.print(" >" , .{});
        }

        pub fn free(self : *OFunction , gc : *Gc) void {
            self.ins.free(); 
            gc.getAlc().destroy(self);
            
        }
    };

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

        pub inline fn parent(self : *OString) *PObj{
            return @ptrCast(self);
        }
    };

};



