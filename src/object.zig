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
const ParentLink = value.ParentLink;
const PValue = value.PValue;
const utils = @import("utils.zig");
const Gc = @import("gc.zig").Gc;
const instruction = @import("instruction.zig");
const Instruction = instruction.Instruction;
const writer = @import("writer.zig");
const table = @import("table.zig");
const Bnum = @import("ext/baurinum/src/bnum.zig").Bnum;
const BnName = @import("bengali/names.zig");
const valueerrors = @import("value_errors.zig");
const CopyError = valueerrors.CopyError;

pub const PObj = struct {
    objtype: OType,
    next: ?*PObj,
    isMarked: bool,

    pub const OType = enum {
        Ot_String,
        Ot_Function,
        Ot_NativeFunc,
        Ot_Closure,
        Ot_UpValue,
        Ot_Array,
        Ot_Hmap,
        Ot_Error,
        Ot_BigInt,
        Ot_Module,

        pub fn toSimpleString(self: OType) []const u8 {
            switch (self) {
                .Ot_String => {
                    return BnName.simpleNameObjString;
                },
                .Ot_Function => {
                    return BnName.simpleNameObjFunction;
                },
                .Ot_NativeFunc => return BnName.simpleNameObjNativeFunc,
                .Ot_Closure => return BnName.simpleNameObjClosure,
                .Ot_UpValue => return BnName.simpleNameObjUpvalue,
                .Ot_Array => return BnName.simpleNameObjArray,
                .Ot_Hmap => return BnName.simpleNameObjHmap,
                .Ot_Error => return BnName.simpleNameObjError,
                .Ot_Module => return BnName.simpleNameObjModule,
                .Ot_BigInt => return BnName.simpleNameObjBigint,
            }
        }

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
                .Ot_BigInt => return "OBJ_BIGNUM",
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

    pub fn createCopy(
        self: *PObj,
        gc: *Gc,
        links: ?*ParentLink,
    ) CopyError!*PObj {
        return switch (self.getType()) {
            .Ot_String => self.asString().createCopy(gc, links),
            .Ot_Function => self,
            .Ot_NativeFunc => self,
            .Ot_Closure => self,
            .Ot_UpValue => self,
            .Ot_Array => self.asArray().createCopy(gc, links),
            .Ot_Hmap => self.asHmap().createCopy(gc, links),
            .Ot_Error => self.asOErr().createCopy(gc, links),
            .Ot_BigInt => self.asBigInt().createCopy(gc, links),
            .Ot_Module => self,
        };
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
        if (utils.IS_WASM) {
            return @alignCast(@fieldParentPtr("obj", self));
        } else {
            return @fieldParentPtr("obj", self);
        }
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

    pub fn isBigInt(self: *PObj) bool {
        return self.is(.Ot_BigInt);
    }

    pub fn asString(self: *PObj) *OString {
        return self.child(PObj.OString);
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

    pub fn asBigInt(self: *PObj) *OBigInt {
        return self.child(PObj.OBigInt);
    }

    pub fn asValue(self: *PObj) PValue {
        return PValue.makeObj(self);
    }

    pub fn printObj(self: *PObj, gc: *Gc, link: ?*ParentLink) bool {
        return switch (self.getType()) {
            .Ot_String => self.asString().print(gc, link),
            .Ot_Function => self.asFunc().print(gc, link),
            .Ot_NativeFunc => self.asNativeFun().print(gc, link),
            .Ot_Closure => self.asClosure().print(gc, link),
            .Ot_UpValue => self.asUpvalue().print(gc, link),
            .Ot_Array => self.asArray().print(gc, link),
            .Ot_Hmap => self.asHmap().print(gc, link),
            .Ot_Error => self.asOErr().print(gc, link),
            .Ot_Module => self.asMod().print(gc, link),
            .Ot_BigInt => self.asBigInt().print(gc, link),
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
                return self.asArray().toString(
                    al,
                ) orelse return std.mem.Allocator.Error.OutOfMemory;
            },
            .Ot_String => {
                return self.asString().chars;
            },

            .Ot_Function => {
                return try std.fmt.allocPrint(al, "<func>", .{});
            },

            .Ot_Closure => {
                return try std.fmt.allocPrint(al, "<closure>", .{});
            },

            .Ot_NativeFunc => {
                return try std.fmt.allocPrint(al, "<native fn>", .{});
            },
            .Ot_UpValue => {
                return try std.fmt.allocPrint(al, "<upvalue>", .{});
            },

            .Ot_Hmap => {
                return self.asHmap().toString(
                    al,
                ) orelse return std.mem.Allocator.Error.OutOfMemory;
            },

            else => {
                return try std.fmt.allocPrint(
                    al,
                    "<object : {s}>",
                    .{self.objtype.toString()},
                );
            },
        }

        return try std.fmt.allocPrint(al, "<object>", .{});
    }

    pub const OBigInt = struct {
        obj: PObj,
        ival: *Bnum,

        pub fn initInt(self: *OBigInt, gc: *Gc) bool {
            self.ival = Bnum.new(gc.hal()) catch return false;
            return true;
        }

        pub fn createCopy(
            self: *OBigInt,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            const cop = gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
                return CopyError.BigInt_NewBigInt;
            };
            cop.parent().isMarked = true;
            if (!cop.initInt(gc)) {
                return CopyError.BigInt_InitInt;
            }

            for (self.ival.digits.items) |digit| {
                if (!cop.ival.addDig(gc.hal(), @intCast(digit))) {
                    return CopyError.BigInt_AddDigit;
                }
            }

            cop.parent().isMarked = false;
            return cop.parent();
        }

        pub fn print(self: *OBigInt, gc: *Gc, _: ?*ParentLink) bool {
            const x = self.ival.toString(gc.hal(), true) orelse {
                return false;
            };
            gc.pstdout.print("{s}", .{x}) catch return false;
            gc.hal().free(x);
            return true;
        }

        pub fn parent(self: *OBigInt) *PObj {
            return @ptrCast(self);
        }

        pub fn free(self: *OBigInt, gc: *Gc) void {
            self.ival.free(gc.hal());
            gc.getAlc().destroy(self);
        }
    };

    pub const OModule = struct {
        obj: PObj,
        name: *OString,

        pub fn new(vm: *Vm, gc: *Gc, name: []const u8) ?*OModule {
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

        pub fn createCopy(
            self: *OModule,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            _ = gc;
            return self.parent();
        }

        pub fn free(self: *OModule, gc: *Gc) void {
            gc.getAlc().destroy(self);
        }

        pub fn print(self: *OModule, gc: *Gc, pl: ?*ParentLink) bool {
            gc.pstdout.print("<oMod ", .{}) catch return false;
            _ = self.name.print(gc, pl);
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

        pub fn initU8Args(
            self: *OError,
            gc: *Gc,
            comptime msg: []const u8,
            args: anytype,
        ) bool {
            self.msg = std.fmt.allocPrint(gc.hal(), msg, args) catch return false;
            return true;
        }

        pub fn initU8(self: *OError, gc: *Gc, msg: []const u8) bool {
            self.msg = gc.hal().alloc(u8, msg.len) catch return false;
            @memcpy(self.msg, msg);
            return true;
        }

        pub fn initU32(self: *OError, gc: *Gc, msg: []const u8) bool {
            return self.initU8(gc, msg);
        }

        pub fn createCopy(
            self: *OError,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            const cop = gc.newObj(.Ot_Error, PObj.OError) catch {
                return CopyError.ErrObj_NewErrObj;
            };

            cop.parent().isMarked = true;

            if (!cop.initU8(gc, self.msg)) {
                return CopyError.ErrObj_CopyError;
            }

            cop.parent().isMarked = false;
            return cop.parent();
        }

        pub fn print(self: *OError, gc: *Gc, _: ?*ParentLink) bool {
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

        pub fn createCopy(
            self: *OHmap,
            gc: *Gc,
            links: ?*ParentLink,
        ) CopyError!*PObj {
            const cop = gc.newObj(.Ot_Hmap, PObj.OHmap) catch {
                return CopyError.Hmap_NewHmap;
            };
            cop.parent().isMarked = true;

            cop.init(gc);

            var linked = false;

            if (links) |lnk| {
                linked = true;
                lnk.prev.append(
                    gc.hal(),
                    PValue.makeObj(self.parent()),
                ) catch return CopyError.Hmap_FailedToAppendParentLink;
            }

            var iter = self.values.iterator();

            while (iter.next()) |item| {
                const key = item.key_ptr.*;
                const val = item.value_ptr.*;

                var copiedKey: ?PValue = null;
                var copiedVal: ?PValue = null;

                if (key.isObj()) {
                    if (linked and links.?.exists(key.asObj())) {
                        copiedKey = key;
                    }
                }

                if (copiedKey == null) {
                    copiedKey = key.createCopy(
                        gc,
                        links,
                    ) catch {
                        return CopyError.Hmap_ItemCopyError;
                    };
                }

                if (val.isObj()) {
                    if (linked and links.?.exists(val.asObj())) {
                        copiedVal = val;
                    }
                }

                if (copiedVal == null) {
                    copiedVal = val.createCopy(
                        gc,
                        links,
                    ) catch {
                        return CopyError.Hmap_ItemCopyError;
                    };
                }

                if (!cop.addPair(gc, copiedKey.?, copiedVal.?)) {
                    return CopyError.Hmap_AddPair;
                }
            }

            cop.parent().isMarked = false;
            return cop.parent();
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

        pub fn toString(self: *OHmap, al: std.mem.Allocator) ?[]u8 {
            var totalLen: usize = 3; //comma (2) + lbrack (1) + rbrack(2)

            var ite = self.values.iterator();
            while (ite.next()) |x| {
                const k = x.key_ptr.*.toString(al) catch return null;
                totalLen += k.len + 2;
                const v = x.value_ptr.*.toString(al) catch return null;
                totalLen += v.len + 2;
                al.free(k);
                al.free(v);
            }

            const sarr = al.alloc(u8, totalLen) catch return null;

            var ptr: [*]u8 = sarr.ptr;

            @memcpy(ptr, "{ ");
            ptr += 2;

            var iter = self.values.iterator();
            while (iter.next()) |x| {
                const k = x.key_ptr.*.toString(al) catch return null;
                @memcpy(ptr, k);
                ptr += k.len;
                @memcpy(ptr, ": ");
                ptr += 2;
                const v = x.value_ptr.*.toString(al) catch return null;
                @memcpy(ptr, v);
                ptr += v.len;
                @memcpy(ptr, ", ");
                ptr += 2;
                totalLen += v.len + 2;
                al.free(k);
                al.free(v);
            }
            @memcpy(ptr, "}");
            ptr += 1;

            return sarr;
        }

        pub fn print(self: *OHmap, gc: *Gc, links: ?*ParentLink) bool {
            gc.pstdout.print("{{", .{}) catch return false;
            const lastIndex = self.values.count();
            var i: isize = 1;
            var ite = self.values.iterator();

            var linked = false;

            if (links) |lnk| {
                linked = true;
                lnk.prev.append(
                    gc.hal(),
                    PValue.makeObj(self.parent()),
                ) catch return false;
            }
            while (ite.next()) |item| {
                const key = item.key_ptr.*;
                const val = item.value_ptr.*;
                if (key.isObj()) {
                    if (linked and links.?.exists(key.asObj())) {
                        gc.pstdout.print("K{{...}}", .{}) catch return false;
                    } else {
                        if (!key.printVal(gc, links)) return false;
                    }
                } else {
                    if (!key.printVal(gc, links)) return false;
                }

                gc.pstdout.print(": ", .{}) catch return false;

                if (val.isObj()) {
                    if (linked and links.?.exists(val.asObj())) {
                        gc.pstdout.print("{{...}}", .{}) catch return false;
                    } else {
                        if (!val.printVal(gc, links)) return false;
                    }
                } else {
                    if (!val.printVal(gc, links)) return false;
                }

                if (i < lastIndex) {
                    gc.pstdout.print(", ", .{}) catch return false;
                }

                i += 1;
            }
            gc.pstdout.print("}}", .{}) catch return false;

            if (linked) {
                _ = links.?.prev.pop();
            }

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

        pub fn createCopy(
            self: *OArray,
            gc: *Gc,
            links: ?*ParentLink,
        ) CopyError!*PObj {
            const cop = gc.newObj(OType.Ot_Array, PObj.OArray) catch {
                return CopyError.Arr_FailedToCreateNewArray;
            };
            cop.parent().isMarked = true;
            cop.init();
            var linked = false;

            if (links) |lnk| {
                linked = true;
                lnk.prev.append(
                    gc.hal(),
                    PValue.makeObj(self.parent()),
                ) catch return CopyError.Arr_FailedToAppendParentLink;
            }

            for (self.values.items) |item| {
                var copiedItem: PValue = undefined;
                if (item.isObj()) {
                    if (linked and links.?.exists(item.asObj())) {
                        copiedItem = item;
                    } else {
                        copiedItem = item.createCopy(
                            gc,
                            links,
                        ) catch {
                            return CopyError.Arr_ItemCopyError;
                        };
                    }
                } else {
                    copiedItem = item.createCopy(
                        gc,
                        links,
                    ) catch {
                        return CopyError.Arr_ItemCopyError;
                    };
                }

                if (!cop.addItem(gc, copiedItem)) {
                    return CopyError.Arr_InsertItems;
                }
            }
            if (linked) {
                _ = links.?.prev.pop();
            }
            cop.parent().isMarked = false;
            return cop.parent();
        }

        pub fn print(self: *OArray, gc: *Gc, links: ?*ParentLink) bool {
            gc.pstdout.print("[", .{}) catch return false;
            const lastIndex = self.values.items.len;
            var linked = false;

            if (links) |lnk| {
                linked = true;
                lnk.prev.append(
                    gc.hal(),
                    PValue.makeObj(self.parent()),
                ) catch return false;
            }

            var i: isize = 1;
            for (self.values.items) |val| {
                if (val.isObj()) {
                    if (linked and links.?.exists(val.asObj())) {
                        gc.pstdout.print("[...]", .{}) catch return false;
                    } else {
                        if (!val.printVal(gc, links)) return false;
                    }
                } else {
                    if (!val.printVal(gc, links)) return false;
                }

                if (i < lastIndex) {
                    gc.pstdout.print(", ", .{}) catch return false;
                }

                i += 1;
            }
            gc.pstdout.print("]", .{}) catch return false;

            if (linked) {
                _ = links.?.prev.pop();
            }

            return true;
        }

        pub fn toString(self: *OArray, al: std.mem.Allocator) ?[]u8 {
            var totalLen: usize = 3; //comma (2) + lbrack (1) + rbrack(2)

            for (self.values.items) |val| {
                const x = val.toString(al) catch return null;
                totalLen += x.len + 2;
                al.free(x);
            }

            const sarr = al.alloc(u8, totalLen) catch return null;

            var ptr: [*]u8 = sarr.ptr;

            @memcpy(ptr, "[ ");
            ptr += 2;

            for (self.values.items) |val| {
                const x = val.toString(al) catch return null;

                @memcpy(ptr, x);
                ptr += x.len;
                @memcpy(ptr, ", ");
                ptr += 2;

                al.free(x);
            }

            @memcpy(ptr, "]");
            ptr += 1;

            return sarr;
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
                const e = self.values.items[end];
                const s = self.values.items[start];
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

        pub fn createCopy(
            self: *OUpValue,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            _ = gc;
            return self.parent();
        }

        pub fn print(self: *OUpValue, gc: *Gc, _: ?*ParentLink) bool {
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

        pub fn createCopy(
            self: *OClosure,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            _ = gc;
            return self.parent();
        }

        pub fn print(self: *OClosure, gc: *Gc, pl: ?*ParentLink) bool {
            gc.pstdout.print("<cls ", .{}) catch return false;
            if (!self.function.print(gc, pl)) return false;
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

        pub fn createCopy(
            self: *ONativeFunction,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            _ = gc;
            return self.parent();
        }

        pub fn print(self: *ONativeFunction, gc: *Gc, _: ?*ParentLink) bool {
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

        pub fn createCopy(
            self: *OFunction,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            _ = gc;
            return self.parent();
        }

        pub inline fn parent(self: *OFunction) *PObj {
            return @ptrCast(self);
        }

        pub fn getName(self: *OFunction) ?[]const u8 {
            if (self.name) |name| {
                if (name.len < 0) {
                    return null;
                } else {
                    return name.chars[0..name.chars.len];
                }
            } else if (self.fromMod) {
                return "<mod>";
                //return &[_]u32{ '<', 'm', 'o', 'd', '>' };
            } else {
                return "<script>";
                //return &[_]u32{ '<', 's', 'c', 'r', 'i', 'p', 't', '>' };
            }
        }

        pub fn print(self: *OFunction, gc: *Gc, _: ?*ParentLink) bool {
            gc.pstdout.print("<Fun ", .{}) catch return false;
            if (self.getName()) |n| {
                gc.pstdout.print("{s}", .{n}) catch return false;
                //utils.printu32(n, gc.pstdout);
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
        chars: []u8,
        hash: u32,
        len: isize,

        fn allocate(gc: *Gc, chars: []const u8) !*OString {
            const obj = try PObj.create(gc, PObj.OString, .Ot_String);
            const str = obj.asString();
            const temp_chars = try gc.getAlc().alloc(u8, chars.len);
            str.len = @intCast(chars.len);
            str.hash = utils.hashChars(chars) catch 0;
            @memcpy(temp_chars.ptr, chars);
            str.chars = temp_chars;

            return str;
        }

        pub fn createCopy(
            self: *OString,
            gc: *Gc,
            _: ?*ParentLink,
        ) CopyError!*PObj {
            const str = gc.copyString(self.chars, @intCast(self.len)) catch {
                return CopyError.String_NewString;
            };
            return str.parent();
        }

        pub fn free(self: *OString, gc: *Gc) void {
            gc.getAlc().free(self.chars);
            gc.getAlc().destroy(self);
        }

        pub fn printWithoutQuotes(self: *OString, gc: *Gc) bool {
            if (self.len < 0) {
                gc.pstdout.print("0x{x}", .{@intFromPtr(self)}) catch return false;
                return true;
            }
            gc.pstdout.print("{s}", .{self.chars}) catch return false;
            //utils.printu32(self.chars, gc.pstdout);
            return true;
        }

        pub fn print(self: *OString, gc: *Gc, _: ?*ParentLink) bool {
            gc.pstdout.print("\"", .{}) catch return false;
            if (!self.printWithoutQuotes(gc)) return false;
            gc.pstdout.print("\"", .{}) catch return false;

            return true;
        }

        pub fn size(self: *OString) usize {
            var total: usize = @sizeOf(OString);
            total += self.chars.len + 1 * @sizeOf(u8);

            total += @sizeOf(@TypeOf(self.len));
            return total;
        }

        pub inline fn parent(self: *OString) *PObj {
            return @ptrCast(self);
        }
    };
};
