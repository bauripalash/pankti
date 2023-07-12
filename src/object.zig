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
const writer = @import("writer.zig");
const table = @import("table.zig");

pub const PObj = struct {
    objtype: OType,
    next: ?*PObj,
    isMarked: bool,

    pub const OType = enum(u8) {
        Ot_String,
        Ot_Function,
        Ot_NativeFunc,
        Ot_Closure,
        Ot_UpValue,
        Ot_Array,
        Ot_Hmap,
        Ot_Error,
        Ot_Module,

        pub fn toString(self: OType) []const u8 {
            switch (self) {
                .Ot_String => {
                    return "OBJ_STRING";
                },
                .Ot_Function => {
                    return "OBJ_FUNC";
                },
                .Ot_NativeFunc => return "OBJ_NATIVE",
                .Ot_Closure => return "OBJ_CLOSURE",
                .Ot_UpValue => return "OBJ_UPVALUE",
                .Ot_Array => return "OBJ_ARRAY",
                .Ot_Hmap => return "OBJ_HMAP",
                .Ot_Error => return "OBJ_ERROR",
                .Ot_Module => return "OBJ_MODULE",
            }
        }
    };

    pub fn create(gc: *Gc, comptime T: type, objtype: OType) !*PObj {
        const ptr = try gc.getAlc().create(T);
        ptr.obj = PObj{
            .next = gc.objects,
            .objtype = objtype,
            .isMarked = false,
        };

        gc.objects = &ptr.obj;

        return &ptr.obj;
    }

    pub fn isEqual(self: *PObj, other: *PObj) bool {
        if (self.getType() != other.getType()) {
            return false;
        }

        if (self.isString()) {
            return self.asString().hash == other.asString().hash;
        }

        return false;
    }

    pub fn child(self: *PObj, comptime ChildType: type) *ChildType {
        return @fieldParentPtr(ChildType, "obj", self);
    }

    pub fn getType(self: *PObj) OType {
        return self.objtype;
    }

    pub fn is(self: *PObj, tp: OType) bool {
        return self.getType() == tp;
    }

    pub fn isString(self: *PObj) bool {
        return self.is(.Ot_String);
    }

    pub fn isArray(self: *PObj) bool {
        return self.is(.Ot_Array);
    }

    pub fn isHmap(self: *PObj) bool {
        return self.is(.Ot_Hmap);
    }

    pub fn isOError(self: *PObj) bool {
        return self.is(.Ot_Error);
    }

    pub fn isMod(self: *PObj) bool {
        return self.is(.Ot_Module);
    }

    pub fn asString(self: *PObj) *OString {
        return @fieldParentPtr(OString, "obj", self);
    }

    pub fn asFunc(self: *PObj) *OFunction {
        return self.child(PObj.OFunction);
    }

    pub fn asNativeFun(self: *PObj) *ONativeFunction {
        return self.child(PObj.ONativeFunction);
    }

    pub fn asClosure(self: *PObj) *OClosure {
        return self.child(PObj.OClosure);
    }

    pub fn asUpvalue(self: *PObj) *OUpValue {
        return self.child(PObj.OUpValue);
    }

    pub fn asArray(self: *PObj) *OArray {
        return self.child(PObj.OArray);
    }

    pub fn asHmap(self: *PObj) *OHmap {
        return self.child(PObj.OHmap);
    }

    pub fn asOErr(self: *PObj) *OError {
        return self.child(PObj.OError);
    }

    pub fn asMod(self: *PObj) *OModule {
        return self.child(PObj.OModule);
    }

    pub fn free(self: *PObj, vm: *Vm) void {
        switch (self.objtype) {
            .Ot_String => self.asString().free(vm),
            else => {},
        }
    }
    pub fn asValue(self: *PObj) PValue {
        return PValue.makeObj(self);
    }

    pub fn printObj(self: *PObj, gc: *Gc) bool {
        return switch (self.getType()) {
            .Ot_String => self.asString().print(gc),
            .Ot_Function => self.asFunc().print(gc),
            .Ot_NativeFunc => self.asNativeFun().print(gc),
            .Ot_Closure => self.asClosure().print(gc),
            .Ot_UpValue => self.asUpvalue().print(gc),
            .Ot_Array => self.asArray().print(gc),
            .Ot_Hmap => self.asHmap().print(gc),
            .Ot_Error => self.asOErr().print(gc),
            .Ot_Module => self.asMod().print(gc),
        };
    }

    pub fn getLen(self: *PObj) ?usize {
        switch (self.getType()) {
            .Ot_String => {
                return @intCast(self.asString().len);
            },
            .Ot_Hmap => {
                return self.asHmap().count;
            },
            .Ot_Array => {
                return self.asArray().count;
            },

            else => {},
        }
        return null;
    }

    pub fn toString(self: *PObj, al: std.mem.Allocator) ![]u8 {
        switch (self.getType()) {
            .Ot_Array => {
                return try utils.u32tou8(&[_]u32{ '[', 'a', ']' });
            },
            .Ot_String => {
                return try utils.u32tou8(self.asString().chars, al);
            },

            .Ot_Function => {
                return try utils.u32tou8(&[_]u32{ '<', 'f', 'n', '>' }, al);
            },

            .Ot_Closure => {
                return try utils.u32tou8(
                    &[_]u32{ 'c', 'l', 'o', 's', 'u', 'r', 'e' },
                    al,
                );
            },

            .Ot_NativeFunc => {
                return try utils.u32tou8(
                    &[_]u32{
                        '<',
                        'n',
                        'a',
                        't',
                        'i',
                        'v',
                        ' ',
                        'f',
                        'n',
                        '>',
                    },
                    al,
                );
            },
            .Ot_UpValue => {
                return try utils.u32tou8(
                    &[_]u32{ 'u', 'p', 'v', 'a', 'l', 'u', 'e' },
                    al,
                );
            },
        }

        return try utils.u32tou8([_]u32{'_'}, al);
    }

    pub const OModule = struct {
        obj: PObj,
        name: *OString,

        pub fn new(vm: *Vm, gc: *Gc, name: []const u32) ?*OModule {
            const om: *OModule = gc.newObj(.Ot_Module, PObj.OModule) catch return null;
            om.name = undefined;

            vm.stack.push(PValue.makeObj(om.parent())) catch return null;

            om.name = gc.copyString(name, @intCast(name.len)) catch {
                _ = vm.stack.pop() catch return null;
                return null;
            };

            _ = vm.stack.pop() catch return null;
            return om;
        }

        pub fn free(self: *OModule, gc: *Gc) void {
            gc.getAlc().destroy(self);
        }

        pub fn print(self: *OModule, gc: *Gc) bool {
            gc.pstdout.print("<oMod ", .{}) catch return false;
            _ = self.name.print(gc);
            gc.pstdout.print(" >", .{}) catch return false;
            return true;
        }

        pub fn parent(self: *OModule) *PObj {
            return @ptrCast(self);
        }
    };
    pub const OError = struct {
        obj: PObj,
        msg: []u8,

        pub fn initU8(self: *OError, gc: *Gc, msg: []const u8) bool {
            self.msg = gc.hal().alloc(u8, msg.len) catch return false;
            @memcpy(self.msg, msg);
            return true;
        }

        pub fn initU32(self: *OError, gc: *Gc, msg: []const u32) bool {
            const msgU8 = utils.u32tou8(msg, gc.hal()) catch return false;
            self.initU8(gc, msgU8);
            gc.hal().free(msgU8);
            return true;
        }

        pub fn print(self: *OError, gc: *Gc) bool {
            gc.pstdout.print("{s}", .{self.msg}) catch return false;
            return true;
        }

        pub fn free(self: *OError, gc: *Gc) void {
            gc.hal().free(self.msg);
            gc.getAlc().destroy(self);
        }

        pub fn parent(self: *OError) *PObj {
            return @ptrCast(self);
        }
    };

    pub const OHmap = struct {
        obj: PObj,
        values: table.MapTable(),
        count: usize,

        pub fn init(self: *OHmap, gc: *Gc) void {
            _ = gc;
            self.count = 0;
            self.values = table.MapTable(){};
        }

        pub fn addPair(self: *OHmap, gc: *Gc, k: PValue, v: PValue) bool {
            //std.debug.print("-->{any}{any}\n" , .{k , v});
            self.values.put(gc.hal(), k, v) catch return false;
            self.count += 1;
            return true;
        }

        pub fn getValue(self: *OHmap, k: PValue) ?PValue {
            if (self.values.get(k)) |v| {
                return v;
            } else {
                return null;
            }
        }

        pub fn print(self: *OHmap, gc: *Gc) bool {
            gc.pstdout.print("{{ ", .{}) catch return false;
            var ite = self.values.iterator();
            while (ite.next()) |item| {
                if (!item.key_ptr.*.printVal(gc)) return false;
                gc.pstdout.print(": ", .{}) catch return false;
                if (!item.value_ptr.*.printVal(gc)) return false;

                gc.pstdout.print(", ", .{}) catch return false;
            }
            gc.pstdout.print("}}", .{}) catch return false;

            return true;
        }

        pub fn free(self: *OHmap, gc: *Gc) void {
            self.values.deinit(gc.hal());
            self.count = 0;
            gc.getAlc().destroy(self);
        }

        pub fn parent(self: *OHmap) *PObj {
            return @ptrCast(self);
        }
    };
    pub const OArray = struct {
        obj: PObj,
        values: std.ArrayListUnmanaged(PValue),
        count: usize,

        pub fn init(self: *OArray) void {
            self.count = 0;
            self.values = std.ArrayListUnmanaged(PValue){};
        }

        pub fn print(self: *OArray, gc: *Gc) bool {
            gc.pstdout.print("[ ", .{}) catch return false;
            for (self.values.items) |val| {
                if (!val.printVal(gc)) return false;
                gc.pstdout.print(", ", .{}) catch return false;
            }
            gc.pstdout.print("]", .{}) catch return false;

            return true;
        }

        pub fn addItem(self: *OArray, gc: *Gc, item: PValue) bool {
            self.values.append(gc.hal(), item) catch return false;
            self.count += 1;
            return true;
        }

        pub fn popItem(self: *OArray) PValue {
            self.count -= 1;
            return self.values.pop();
        }

        pub fn reverseItems(self: *OArray) void {
            var start: usize = 0;
            var end = self.count - 1;

            while (start < end) {
                var e = self.values.items[end];
                var s = self.values.items[start];
                self.values.items[start] = e;
                self.values.items[end] = s;
                start += 1;
                end -= 1;
            }
        }

        pub fn free(self: *OArray, gc: *Gc) void {
            self.values.deinit(gc.hal());
            self.count = 0;
            gc.getAlc().destroy(self);
        }

        pub fn parent(self: *OArray) *PObj {
            return @ptrCast(self);
        }
    };

    pub const OUpValue = struct {
        obj: PObj,
        location: *PValue,
        next: ?*OUpValue,
        closed: PValue,

        pub fn init(self: *OUpValue, val: *PValue) void {
            self.location = val;
            self.next = null;
            self.closed = PValue.makeNil();
        }

        pub fn print(self: *OUpValue, gc: *Gc) bool {
            _ = self;
            gc.pstdout.print("upvalue", .{}) catch return false;

            return true;
        }

        pub fn free(self: *OUpValue, gc: *Gc) void {
            gc.getAlc().destroy(self);
        }

        pub fn parent(self: *OUpValue) *PObj {
            return @ptrCast(self);
        }
    };

    pub const OClosure = struct {
        obj: PObj,
        function: *OFunction,
        upvalues: [*]*OUpValue,
        upc: u32,
        globOwner: u32,
        globals: ?*table.PankTable(),

        pub fn init(self: *OClosure, gc: *Gc, func: *OFunction) !void {
            const upvalues = try gc.getAlc().alloc(?*OUpValue, func.upvCount);
            for (upvalues) |*v| {
                v.* = std.mem.zeroes(?*OUpValue);
            }
            const ptr: [*]*OUpValue = @ptrCast(upvalues);
            self.function = func;
            self.upvalues = ptr;
            self.upc = func.upvCount;
            self.globals = null;
            self.globOwner = 0;
        }

        pub fn new(gc: *Gc, func: *OFunction) !*PObj.OClosure {
            const upvalues = try gc.getAlc().alloc(?*OUpValue, func.upvCount);
            for (upvalues) |*v| {
                v.* = std.mem.zeroes(?*OUpValue);
            }
            const cl = try gc.newObj(.Ot_Closure, PObj.OClosure);
            //cl.parent().isMarked = true;
            const ptr: [*]*OUpValue = @ptrCast(upvalues);
            cl.upvalues = ptr;
            cl.upc = func.upvCount;
            cl.function = func;
            cl.globals = null;
            cl.globOwner = 0;
            //cl.parent().isMarked = false;
            return cl;
        }

        pub fn print(self: *OClosure, gc: *Gc) bool {
            gc.pstdout.print("<cls ", .{}) catch return false;
            if (!self.function.print(gc)) return false;
            gc.pstdout.print(" >", .{}) catch return false;

            return true;
        }

        pub fn parent(self: *OClosure) *PObj {
            return @ptrCast(self);
        }

        pub fn free(self: *OClosure, gc: *Gc) void {
            gc.getAlc().free(self.upvalues[0..self.upc]);
            gc.getAlc().destroy(self);
        }
    };

    pub const ONativeFunction = struct {
        obj: PObj,
        func: NativeFn,

        pub const NativeFn = *const fn (v: *Vm, u8, []PValue) PValue;

        pub fn init(self: *ONativeFunction, func: NativeFn) void {
            self.func = func;
        }

        pub fn print(self: *ONativeFunction, gc: *Gc) bool {
            _ = self;
            gc.pstdout.print("<Native Fn>", .{}) catch return false;

            return true;
        }

        pub fn parent(self: *ONativeFunction) *PObj {
            return @ptrCast(self);
        }

        pub fn free(self: *ONativeFunction, gc: *Gc) void {
            gc.getAlc().destroy(self);
        }
    };

    pub const OFunction = struct {
        obj: PObj,
        arity: u8,
        name: ?*OString,
        upvCount: u32,
        ins: Instruction,
        fromMod: bool,

        pub fn init(self: *OFunction, gc: *Gc) void {
            self.arity = 0;
            self.name = null;
            self.upvCount = 0;
            self.ins = Instruction.init(gc);
            self.fromMod = false;
        }

        pub inline fn parent(self: *OFunction) *PObj {
            return @ptrCast(self);
        }

        pub fn getName(self: *OFunction) ?[]const u32 {
            if (self.name) |name| {
                if (name.len < 0) {
                    return null;
                } else {
                    return name.chars[0..name.chars.len];
                }
            } else if (self.fromMod) {
                return &[_]u32{ '<', 'm', 'o', 'd', '>' };
            } else {
                return &[_]u32{ '<', 's', 'c', 'r', 'i', 'p', 't', '>' };
            }
        }

        pub fn print(self: *OFunction, gc: *Gc) bool {
            gc.pstdout.print("<Fun ", .{}) catch return false;
            if (self.getName()) |n| {
                utils.printu32(n, gc.pstdout);
            } else {
                gc.pstdout.print("0x{x}", .{@intFromPtr(self.name.?)}) catch
                    return false;
            }
            gc.pstdout.print(" >", .{}) catch return false;
            return true;
        }

        pub fn free(self: *OFunction, gc: *Gc) void {
            self.ins.free();
            gc.getAlc().destroy(self);
        }
    };

    pub const OString = struct {
        obj: PObj,
        chars: []u32,
        hash: u32,
        len: isize,

        fn allocate(gc: *Gc, chars: []const u32) !*OString {
            const obj = try PObj.create(gc, PObj.OString, .Ot_String);
            const str = obj.asString();
            var temp_chars = try gc.getAlc().alloc(u32, chars.len);
            str.len = @intCast(chars.len);
            str.hash = utils.hashU32(chars) catch 0;
            @memcpy(temp_chars.ptr, chars);
            str.chars = temp_chars;

            return str;
        }

        pub fn free(self: *OString, gc: *Gc) void {
            gc.getAlc().free(self.chars);
            gc.getAlc().destroy(self);
        }

        pub fn print(self: *OString, gc: *Gc) bool {
            if (self.len < 0) {
                gc.pstdout.print("0x{x}", .{@intFromPtr(self)}) catch return false;
                return true;
            }

            //gc.pstdout.print("\"" , .{}) catch return false;
            utils.printu32(self.chars, gc.pstdout);

            //gc.pstdout.print("\"" , .{}) catch return false;
            return true;
        }

        pub fn size(self: *OString) usize {
            var total: usize = @sizeOf(OString);
            total += self.chars.len + 1 * @sizeOf(u32);
            //+1 is for self.hash each a u32

            total += @sizeOf(@TypeOf(self.len));
            return total;
        }

        pub inline fn parent(self: *OString) *PObj {
            return @ptrCast(self);
        }
    };
};
