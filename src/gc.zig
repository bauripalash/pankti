//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const Allocator = std.mem.Allocator;
const PObj = @import("object.zig").PObj;
const value_zig = @import("value.zig");
const PValue = value_zig.PValue;
const PValueType = value_zig.PValueType;
const table = @import("table.zig");
const utils = @import("utils.zig");
const flags = @import("flags.zig");
const ansicolors = @import("ansicolors.zig");
const vm = @import("vm.zig");
const compiler = @import("compiler.zig");

pub const Gc = struct {
    internal_al: Allocator,
    al: Allocator,
    handyal : Allocator,
    objects: ?*PObj,
    strings: table.PankTable(),
    globals: table.PankTable(),
    openUps: ?*PObj.OUpValue,
    alocAmount: usize,
    stack: ?*vm.VStack,
    callstack: ?*vm.CallStack,
    compiler : ?*compiler.Compiler,
    grayStack : std.ArrayListUnmanaged(*PObj),

    const Self = @This();

    pub fn new(al: Allocator , handlyal : Allocator ) !*Gc {
        const newgc = try al.create(Self);
        newgc.* = .{
            .internal_al = al,
            .al = undefined,
            .handyal = handlyal,
            .strings = table.PankTable(){},
            .globals = table.PankTable(){},
            .objects = null,
            .openUps = null,
            .alocAmount = 0,
            .stack = null,
            .callstack = null,
            .compiler = null,
            .grayStack = std.ArrayListUnmanaged(*PObj){},
        };

        return newgc;
    }

    pub fn boot(self: *Self) void {
        self.al = self.allocator();
    }

    pub inline fn allocator(self: *Self) Allocator {
        return .{ .ptr = self, .vtable = comptime &Allocator.VTable{
            .alloc = allocImpl,
            .free = freeImpl,
            .resize = resizeImpl,
        } };
    }

    pub fn allocImpl(ptr: *anyopaque, len: usize, ptr_align: u8, ret_addr: usize) ?[*]u8 {
        const self: *Gc = @ptrCast(@alignCast(ptr));
        const bts = self.internal_al.rawAlloc(len, ptr_align, ret_addr);
        self.alocAmount += len;
        //self.collect() catch return bts;
        std.debug.print("ALOC SIZE ->{d}bytes\n" , .{len});
        return bts;
    }

    pub fn freeImpl(ptr: *anyopaque, buf: []u8, bufalign: u8, ret_addr: usize) void {
        const self: *Gc = @ptrCast(@alignCast(ptr));
        self.alocAmount -= buf.len;
        self.internal_al.rawFree(buf, bufalign, ret_addr);
    }

    pub fn resizeImpl(ptr: *anyopaque, buf: []u8, bufalign: u8, newlen: usize, ret_addr: usize) bool {
        const self: *Gc = @ptrCast(@alignCast(ptr));
        if (buf.len > newlen) {
            self.alocAmount = (self.alocAmount - buf.len) + newlen;
        } else if (buf.len < newlen) {
            self.alocAmount = (self.alocAmount - buf.len) + newlen;
        }
        return self.internal_al.rawResize(buf, bufalign, newlen, ret_addr);
    }

    pub fn getIntAlc(self: *Self) Allocator {
        return self.inernal_al;
    }

    pub fn getAlc(self: *Self) Allocator {
        return self.al;
    }

    pub fn hal(self : *Self) Allocator {
        return self.handyal;
    }

    pub fn newObj(self: *Self, otype: PObj.OType, comptime ParentType: type) !*ParentType {
        const ptr = try self.al.create(ParentType);
        ptr.parent().objtype = otype;
         if (flags.DEBUG_GC) {
            ansicolors.TermColor('b');
            std.debug.print("[GC] (0x{x}) New Object: {s}", .{ @intFromPtr(ptr), ptr.parent().objtype.toString() });

            ansicolors.ResetColor();
            std.debug.print("\n", .{});
        }
        ptr.parent().isMarked = false;
        ptr.parent().next = self.objects;
        self.objects = ptr.parent();

        

        return ptr;
    }

    pub fn newString(self: *Self, chars: []u32, len: u32) !*PObj.OString {
        var ptr = try self.newObj(.Ot_String, PObj.OString);
        ptr.chars = chars;
        ptr.len = len;
        ptr.obj.isMarked = false;
        ptr.hash = try utils.hashU32(chars);

        try self.strings.put(self.hal(), ptr , PValue.makeNil());

        return ptr;
    }

    pub fn copyString(self: *Gc, chars: []const u32, len: u32) !*PObj.OString {

        if (table.getString(self.strings, try utils.hashU32(chars), len)) |interned| {
            return interned;
        }

        const mem_chars = try self.al.alloc(u32, len);
        @memcpy(mem_chars, chars);

        return self.newString(mem_chars, len);
    }

    pub fn takeString(self : *Gc , chars : []const u32 , len : u32) !*PObj.OString{
        if (self.strings.get(chars)) |interned| {
            try self.getAlc().free(chars);
            return interned;
        }

        return try self.newString(chars, len);


    }

    pub fn freeSingleObject(self: *Self, obj: *PObj) void {
        if (flags.DEBUG_GC) {
            ansicolors.TermColor('p');
            std.debug.print("[GC] (0x{x}) Free Object: {s} : ", .{ @intFromPtr(obj), obj.objtype.toString() });
            obj.printObj();
            ansicolors.ResetColor();
            std.debug.print("\n", .{});
        }
        switch (obj.objtype) {

            .Ot_Function => {
                const fnObj = obj.child(PObj.OFunction);
                fnObj.free(self);
            },
            .Ot_String => {
                const str_obj = obj.child(PObj.OString);
                str_obj.free(self);
            },

            .Ot_NativeFunc => {
                const nfObj = obj.asNativeFun();
                nfObj.free(self);
            },

            .Ot_Closure => {
                const cl = obj.asClosure();
                cl.free(self);
            },

            .Ot_UpValue => {
                obj.asUpvalue().free(self);
            },
        }

        return;
    }

    pub fn freeObjects(self: *Self) void {
        var object : ?*PObj = self.objects;

        while (object) |obj| {
            const next = obj.next;
            self.freeSingleObject(obj);
            object = next;
        }
    }

    pub fn freeGc(self : *Self , al : Allocator) void{
        al.destroy(self);
    }
    pub fn free(self: *Self) void {
        if (flags.DEBUG and flags.DEBUG_GC) {
            //std.debug.print("TOTAL BYTES ALLOCATED-> {d}bytes\n" , .{self.alocAmount});
        }
        self.freeObjects();
        self.strings.deinit(self.hal());
        self.globals.deinit(self.hal());
        self.grayStack.deinit(self.hal());
        //self.getAlc().destroy(self);
    }

   };
