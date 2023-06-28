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
        //std.debug.print("allocator -> {any}\n" , .{self.getAlc()});
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
        self.collect() catch return bts;
        //std.debug.print("ALOC SIZE ->{d}bytes\n" , .{len});
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
            .Ot_String => {
                const str_obj = obj.child(PObj.OString);
                //if (flags.DEBUG_GC) {
                //str_obj.print();
                //std.debug.print("{s}\n" , .{ansicolors.ANSI_COLOR_RESET});
                //}

                str_obj.free(self);
            },

            .Ot_Function => {
                const fnObj = obj.child(PObj.OFunction);
                fnObj.free(self);
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

    pub fn free(self: *Self) void {
        if (flags.DEBUG and flags.DEBUG_GC) {
            //std.debug.print("TOTAL BYTES ALLOCATED-> {d}bytes\n" , .{self.alocAmount});
        }
        self.freeObjects();
        self.strings.deinit(self.hal());
        self.globals.deinit(self.hal());
        self.grayStack.deinit(self.getAlc());
        self.getAlc().destroy(self);
    }

    pub fn collect(self: *Self) !void {
        if (flags.DEBUG and flags.DEBUG_GC) {
            ansicolors.TermColor('r');
            std.debug.print("[GC] () GC Process Begin\n", .{});
            ansicolors.ResetColor();
        }
        try self.markRoots();
        try self.traceRefs();
        try self.removeStrings();
        if (flags.DEBUG and flags.DEBUG_GC) {
            ansicolors.TermColor('b');
            std.debug.print("[GC] () Sweeping \n", .{});
            ansicolors.ResetColor();
        }


        try self.sweep();
        if (flags.DEBUG and flags.DEBUG_GC) {
            ansicolors.TermColor('b');
            std.debug.print("[GC] () Finished Sweeping \n", .{});
            ansicolors.ResetColor();
        }
        //
        if (flags.DEBUG and flags.DEBUG_GC) {
            ansicolors.TermColor('r');
            std.debug.print("[GC] () GC Process End\n", .{});
            ansicolors.ResetColor();
        }
    }

    pub fn removeStrings(self : *Self) !void{
        var ite = self.strings.iterator();

        while (ite.next()) |n| {

            std.debug.print("{any}\n" , .{n.value_ptr.*});
            //try self.markObject(n.value_ptr.*.parent());
            if (!n.key_ptr.*.parent().isMarked){
                _ = self.strings.orderedRemove(n.key_ptr.*);
            }
        }
        
    }
    pub fn markCompilerRoots(self : *Self) !void{
        if (self.compiler) |comp| {
         if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Marking Compiler Roots \n", .{});
            }

         var cp : ?*compiler.Compiler = comp;

         while (cp) |c| {
            try self.markObject(c.function.parent());
            if (c.enclosing) |u| {
                cp = u;
            }else {
                cp = null;
            }
         }

        if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Finished Marking Compiler Roots \n", .{});

            }
        }
    }
    pub fn markOpenUpvalues(self : *Self) !void{
        
        if (self.openUps) |openups| {
            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Marking Upvalues \n", .{});
            }
            var up : ?*PObj.OUpValue = openups;

            while (up) |u| {
                try self.markObject(u.parent());
                if (u.next) |n| {
                    up = n;
                } else{
                    up = null;
                }
            }

            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Finished Marking Upvalues \n", .{});
            }
        }

    }
    pub fn markCallFrames(self: *Self) !void {
        if (self.callstack) |cstack| {
            var i: u32 = 0;
            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Marking Callframes [{d}]\n", .{cstack.count});
            }

            while (i < cstack.count) : (i += 1) {
                ansicolors.TermColor('r');
                const closure = cstack.stack[i].closure.parent();
                std.debug.print("[GC] () CallFrame {} ", .{i+1});
                //closure.printObj();
                std.debug.print("\n" , .{});
                ansicolors.ResetColor();

                try self.markObject(closure);
            }

            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Finished Marking Callframes\n", .{});
            }
        }
    }

    fn markStack(self: *Self) !void {
        if (self.stack) |stack| {
            var i: u64 = 0;
            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Marking Stack\n", .{});
            }

            const count = stack.presentcount();
            while (i < count) : (i += 1) {
                const v = stack.stack[i];
                if (flags.DEBUG and flags.DEBUG_GC) {
                    //ansicolors.TermColor('p');
                    //std.debug.print("[GC] (0x{x}) Marking Stack Value: {s} : ", .{ v.data, v.getTypeAsString() });
                    //v.printVal();
                    //ansicolors.ResetColor();
                    //std.debug.print("\n", .{});
                }
                try self.markValue(v);
            }

            if (flags.DEBUG and flags.DEBUG_GC) {
                std.debug.print("[GC] () Finished Marking Stack\n", .{});
            }
        }
    }

    pub fn traceRefs(self : *Self) !void{
        while (self.grayStack.items.len > 0) {
            const obj = self.grayStack.pop();
            try self.paintObject(obj);
        }
    }

    pub fn paintObject(self : *Self , obj : *PObj) !void{
        if (flags.DEBUG and flags.DEBUG_GC and obj.getType() != .Ot_String and obj.getType() != .Ot_NativeFunc) {
            ansicolors.TermColor('c');
            std.debug.print("[GC] () Painting Object : {s} " , .{obj.getType().toString()});
            ansicolors.ResetColor();
    
            obj.printObj();
            std.debug.print("\n" , .{});
        }

        switch (obj.getType()) {
            .Ot_NativeFunc , .Ot_String => {},
            .Ot_Function => {
                const func = obj.asFunc();
                if (func.name) |n| {
                    try self.markObject(n.parent());
                }

                var i : usize = 0;
                while (i < func.ins.cons.items.len) : (i += 1) {
                    const con = func.ins.cons.items[i];
                    try self.markValue(con);
                }
            }, 

            .Ot_UpValue => {
                const up = obj.asUpvalue();
                try self.markValue(up.closed);
            },

            .Ot_Closure => {
                const cl = obj.asClosure();
                try self.markObject(cl.function.parent());
                var i : usize = 0;

                while (i < cl.upc) : (i += 1) {
                    try self.markObject(cl.upvalues[i].parent());
                }
            },

            //else => {},
        }
    }

    pub fn markRoots(self: *Self) !void {
        try self.markStack();
        try self.markCallFrames();
        try self.markOpenUpvalues();

        try self.markGlobals();
        try self.markCompilerRoots();
    }

    pub fn markObject(self: *Self, obj: *PObj) !void {
        if (obj.isMarked) {
            return;
        }

        if (flags.DEBUG and flags.DEBUG_GC) {
            ansicolors.TermColor('g');
            std.debug.print("[GC] (0x{d}) Marking Object " , .{@intFromPtr(obj)});
            obj.printObj();
            std.debug.print("\n" , .{});
            ansicolors.ResetColor();
        }
        obj.isMarked = true;
        try self.grayStack.append(self.getAlc() , obj);
    }

    pub fn markValue(self: *Self, value: PValue) !void {
        if (value.isObj()) {
            try self.markObject(value.asObj());
        }

        return;
    }
    pub fn markGlobals(self: *Self) !void {
        if (flags.DEBUG and flags.DEBUG_GC) {
            //ansicolors.TermColor('g');
            std.debug.print("[GC] () Marking Globals \n" , .{});
        }
        for (self.globals.values()) |v| {
            try self.markValue(v);
        }

        for (self.globals.keys()) |k| {
            try self.markObject(k.parent());
        }

        if (flags.DEBUG and flags.DEBUG_GC) {
            //ansicolors.TermColor('g');
            std.debug.print("[GC] () Finished Marking Globals \n" , .{});
        }
    }

    pub fn sweep(self: *Self) !void {
        var previous: ?*PObj = null;
        var object: ?*PObj = self.objects;

        while (object) |obj| {
            if (obj.isMarked) {
                obj.isMarked = false;
                previous = obj;
                object = obj.next;
            } else {
                const x = obj;
                object = obj.next;

                if (previous) |p| {
                    p.next = object;
                } else {
                    self.objects = object;
                }

                self.freeSingleObject(x);
            }
        }
    }
};
