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
const writer = @import("writer.zig");

const slog: bool = flags.DEBUG and flags.DEBUG_GC;

fn dprint(color: u8, comptime fmt: []const u8, args: anytype) void {
    if (slog) {
        ansicolors.TermColor(color);
        std.debug.print(fmt, args);
        ansicolors.ResetColor();
    }
}

pub const Gc = struct {
    internal_al: Allocator,
    al: Allocator,
    handyal: Allocator,
    objects: ?*PObj,
    strings: table.PankTable(),
    globals: table.PankTable(),
    openUps: ?*PObj.OUpValue,
    alocAmount: usize,
    nextGc : usize,
    stack: ?*vm.VStack,
    callstack: ?*vm.CallStack,
    compiler: ?*compiler.Compiler,
    grayStack: std.ArrayListUnmanaged(*PObj),
    pstdout : writer.PanWriter,
    pstderr : writer.PanWriter,

    const Self = @This();

    pub fn new(al: Allocator, handlyal: Allocator) !*Gc {
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
            .nextGc = 0,
            .stack = null,
            .callstack = null,
            .compiler = null,
            .grayStack = std.ArrayListUnmanaged(*PObj){},
            .pstdout = undefined,
            .pstderr = undefined,
        };

        return newgc;
    }

    pub fn boot(self: *Self , stdout : writer.PanWriter , stderr : writer.PanWriter,) void {
        self.al = self.allocator();
        self.pstdout = stdout;
        self.pstderr = stderr;
    }

    pub inline fn allocator(self: *Self) Allocator {
        return .{ .ptr = self, .vtable = comptime &Allocator.VTable{
            .alloc = allocImpl,
            .free = freeImpl,
            .resize = resizeImpl,
        } };
    }

    pub fn allocImpl(
        ptr: *anyopaque,
        len: usize,
        ptr_align: u8,
        ret_addr: usize,
    ) ?[*]u8 {
        const self: *Gc = @ptrCast(@alignCast(ptr));
        const bts = self.internal_al.rawAlloc(len, ptr_align, ret_addr);
        self.alocAmount += len;
        self.tryCollect();

        return bts;
    }

    pub fn freeImpl(
        ptr: *anyopaque,
        buf: []u8,
        bufalign: u8,
        ret_addr: usize,
    ) void {
        const self: *Gc = @ptrCast(@alignCast(ptr));
        self.alocAmount -= buf.len;
        self.internal_al.rawFree(buf, bufalign, ret_addr);
        
        //self.collect();
    }

    pub fn resizeImpl(
        ptr: *anyopaque,
        buf: []u8,
        bufalign: u8,
        newlen: usize,
        ret_addr: usize,
    ) bool {
        const self: *Gc = @ptrCast(@alignCast(ptr));

        self.alocAmount += newlen - buf.len;
        const result =  self.internal_al.rawResize(buf, bufalign, newlen, ret_addr,);

        if (newlen > buf.len) {
            self.tryCollect();
        }
        return result;
    }

    pub fn getIntAlc(self: *Self) Allocator {
        return self.inernal_al;
    }

    pub fn getAlc(self: *Self) Allocator {
        return self.al;
    }

    pub fn hal(self: *Self) Allocator {
        return self.handyal;
    }

    pub fn newObj(
        self: *Self,
        otype: PObj.OType,
        comptime ParentType: type,
    ) !*ParentType {
        const ptr = try self.al.create(ParentType);
        ptr.parent().objtype = otype;
        if (flags.DEBUG_GC) {
            ansicolors.TermColor('b');

            std.debug.print("[GC] (0x{x}) New Object: {s}", .{
                @intFromPtr(ptr),
                ptr.parent().objtype.toString(),
            });

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
        ptr.len = @intCast(len);
        ptr.obj.isMarked = true;
        ptr.hash = try utils.hashU32(chars);

        try self.strings.put(self.hal(), ptr, PValue.makeNil());
        ptr.obj.isMarked = false;

        return ptr;
    }

    pub fn copyString(self: *Gc, chars: []const u32, len: u32) !*PObj.OString {
        if (table.getString(
            self.strings,
            try utils.hashU32(chars),
            len,
        )) |interned| {
            dprint('b' , "[GC] Returning interned string : " , .{});
            if (slog) {
                interned.print();
            }
            dprint('n' , "\n", .{});
            return interned;
        }

        const mem_chars = try self.getAlc().alloc(u32, len);
        @memcpy(mem_chars, chars);

        return self.newString(mem_chars, len);
    }

    pub fn takeString(self: *Gc, chars: []const u32, len: u32) !*PObj.OString {
        if (self.strings.get(chars)) |interned| {
            try self.getAlc().free(chars);
            return interned;
        }

        return try self.newString(chars, len);
    }

    pub fn printTable(self : *Self , tab : *table.PankTable(), tabname : []const u8) void {
        _ = self;
        var ite = tab.iterator();

        std.debug.print("==== {s} ====\n"  ,.{tabname});
        while (ite.next()) |value| {
            std.debug.print("[" , .{});
            value.key_ptr.*.print();
            std.debug.print("] -> [" , .{});
            value.value_ptr.*.printVal();
            std.debug.print("]\n" , . {});
        }
        std.debug.print("==============\n"  ,.{});
    }

    pub fn freeSingleObject(self: *Self, obj: *PObj) void {
        
        if (flags.DEBUG and (flags.DEBUG_GC or flags.DEBUG_FREE_OBJECTS)) {
            ansicolors.TermColor('p');
            std.debug.print("[GC] (0x{x}) Free Object: {s} : [ ", .{
                @intFromPtr(obj),
                obj.objtype.toString(),
            });
            obj.printObj();
            std.debug.print(" ]\n", .{});
            ansicolors.ResetColor();
        }
        switch (obj.objtype) {
            .Ot_Function => {
                const fnObj = obj.child(PObj.OFunction);
                //self.getAlc().destroy(obj);
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

            .Ot_Array => {
                obj.asArray().free(self);
            },


        }

        return;
    }

    pub fn freeObjects(self: *Self) void {
        var object: ?*PObj = self.objects;

        while (object) |obj| {
            const next = obj.next;
            self.freeSingleObject(obj);
            object = next;
        }
    }

    pub fn freeGc(self: *Self, al: Allocator) void {
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

    pub fn tryCollect(self : *Self) void{
       if ((self.alocAmount > self.nextGc) or flags.STRESS_GC) {
            self.collect();
       } 
    }

    pub fn collect(self: *Self) void {
        //self.grayStack.deinit(self.hal());
        

        dprint('r' , "[GC] Marking Roots\n" , .{});
        self.markRoots();

        dprint('r' , "[GC] Finished Marking Roots\n" , .{});
        dprint('r' , "[GC] Tracing Refs\n" , .{});
        self.traceRefs();
        dprint('r' , "[GC] Finished Tracing Refs\n" , .{});

        
        dprint('r' , "[GC] Cleaning Strings\n" , .{});
            self.removeTableUnpainted(&self.strings);
        dprint('r' , "[GC] Finished Cleaning Strings\n" , .{});
        
        dprint('r' , "[GC] Sweeping\n" , .{});
        self.sweep();
        dprint('r' , "[GC] Finished Sweeping\n" , .{});
    }

    fn removeTableUnpainted(self : *Self , tab : *table.PankTable()) void {
        _ = self;
        var i : usize = 0;
        while (i < tab.count()) : (i+=1) {
            const key = tab.getKeyPtr(tab.keys()[i]);
            if (key) |k| {
                if (!k.*.parent().isMarked) {
                    dprint('p', "[GC] Removing String " , .{});
                    if (slog) {
                        k.*.print();
                    }
                    dprint('p' , "\n" , .{});
                    _ = tab.orderedRemove(k.*);

                }
            }
        }
 

    }

    fn markRoots(self: *Self) void {
        if (self.stack) |stack| {
            dprint('r', "[GC] Marking Stack \n", .{});
            for (0..@intCast(stack.presentcount())) |i| {
                self.markValue(stack.stack[i]);
            }
            dprint('r', "[GC] Finished Marking Stack \n", .{});
        }

        dprint('r', "[GC] Marking Globals \n", .{});
        self.markTable(self.globals);
        dprint('r', "[GC] Finished Marking Globals \n", .{});

        dprint('r', "[GC] Marking CallStack\n", .{});
        const count = self.markCallStack();

        dprint('r', "      [GC] Marked ({}) CallFrames \n", .{count});
        dprint('r', "[GC] Finished Marking CallStack\n", .{});

        dprint('r', "[GC] Marking Open Upvalues\n", .{});
        const ocount = self.markOpenUpvalues();
        dprint('r', "      [GC] Marked ({}) Open Upvalues \n", .{ocount});
        dprint('r', "[GC] Finished Marking Open Upvalues\n", .{});

        dprint('r' , "[GC] Marking Compiler Roots \n" , .{});
        self.markCompilerRoots();
        dprint('r' , "[GC] Finished Marking Compiler Roots \n" , .{});

    }

    fn markTable(self: *Self, tab: table.PankTable()) void {
        var ite = tab.iterator();

        while (ite.next()) |val| {
            self.markObject(val.key_ptr.*.parent());
            //std.debug.print("->{any}" , .{val});
            self.markValue(val.value_ptr.*);
        }
    }

    fn markValue(self: *Self, v: PValue) void {
        if (v.isObj()) {
            self.markObject(v.asObj());
        }
    }

    fn markObject(self: *Self, obj: ?*PObj) void {
        if (obj) |o| {
            if (o.isMarked) { return; }
            dprint('g', "[GC] Marking Object : {s} : [ ", .{
                o.getType().toString(),
            });
            if (slog) {
                o.printObj();
            }
            dprint('g', " ] \n", .{});
            o.isMarked = true;
            self.grayStack.append(self.hal(), o) catch return;
        }
    }

    fn traceRefs(self : *Self) void {
        while (self.grayStack.items.len > 0) {
            const obj = self.grayStack.pop();
            self.paintObject(obj);
        }
    }

    fn paintObject(self : *Self , obj : *PObj) void {
        switch (obj.getType()) {
            .Ot_String , .Ot_NativeFunc => {},
            .Ot_Function => {
                const f = obj.asFunc();
                if (f.name) |name| {

                    self.markObject(name.parent());
                }

                for (f.ins.cons.items) |con| {
                    self.markValue(con);
                }
            },

            .Ot_Array => {
                const arr = obj.asArray();
                
                var i : usize = 0;
                while (i < arr.values.items.len) : ( i += 1 ) {
                    self.markValue(arr.values.items[i]);
                }
            },

            .Ot_UpValue => {
                const o = obj.asUpvalue();
                self.markValue(o.closed);
            },

            .Ot_Closure => {
                const c = obj.asClosure();
                self.markObject(c.function.parent());
                var i : usize = 0;
                while (i < c.upc) : (i+=1) {
                    self.markObject(c.upvalues[i].parent());
                }
            },
            //else => {
            //    return;
            //}
        }
    }

    fn sweep(self : *Self) void {
        var previous : ?*PObj = null;
        var object = self.objects;

        while (object) |obj| {
            if (obj.isMarked) {
                obj.isMarked = false;
                previous = obj;
                object = obj.next;
            } else {
                const ur = obj;
                object = obj.next;

                if (previous) |prev| {
                    prev.next = object;
                } else {
                    self.objects = object;
                }
                
                    self.freeSingleObject(ur);

            }
        }
    }

    fn markCallStack(self: *Self) i32 {
        if (self.callstack) |callstack| {
            var i: usize = 0;
            while (i < callstack.count) : (i += 1) {
                self.markObject(callstack.stack[i].closure.parent());
            }

            return @intCast(i);
        }

        return -1;
    }

    fn markOpenUpvalues(self: *Self) i32 {
        var upv = self.openUps;
        var i: i32 = 0;

        while (upv) |u| {
            upv = u.next;
            self.markObject(u.parent());
            i += 1;
        }

        return i;
    }

    pub fn markCompilerRoots(self: *Self) void {
        if (self.compiler) |scompiler| {
            var comp: ?*compiler.Compiler = scompiler;
            while (comp) |com| {
                self.markObject(com.function.parent());
                comp = com.enclosing;
            }
        }
    }
};
