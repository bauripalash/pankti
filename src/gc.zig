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
const stck = @import("stack.zig");

var DEBUG_GC: bool = false;
var DEBUG: bool = false;
var STRESS_GC: bool = false;
var NO_GC: bool = false;

fn dprint(color: u8, w: std.io.AnyWriter, comptime fmt: []const u8, args: anytype) void {
    ansicolors.TermColor(color, w);
    w.print(fmt, args) catch return;
    ansicolors.ResetColor(w);
}

pub const StdLibMod = struct {
    items: table.PankTable(),
    name: []const u8,
    hash: u32,
    owners: std.ArrayListUnmanaged(u32),
    ownerCount: u32,

    pub fn new() StdLibMod {
        return StdLibMod{
            .items = table.PankTable(){},
            .name = undefined,
            .hash = 0,
            .owners = std.ArrayListUnmanaged(u32){},
            .ownerCount = 0,
        };
    }

    pub fn free(self: *StdLibMod, gc: *Gc) void {
        self.owners.deinit(gc.hal());
        self.items.deinit(gc.hal());
    }
};

pub const StdLibProxy = struct {
    stdmod: *StdLibMod,
    originName: []const u8,
    proxyName: []u8,
    proxyHash: u32,

    pub fn new(hash: u32, proxyname: []u8) StdLibProxy {
        return StdLibProxy{
            .stdmod = undefined,
            .originName = undefined,
            .proxyName = proxyname,
            .proxyHash = hash,
        };
    }
};

pub const Module = struct {
    globals: table.PankTable(),
    stdlibCount: u32,
    stdProxies: std.ArrayListUnmanaged(StdLibProxy),
    frames: stck.CallStack,
    frameCount: u32,
    name: []u8,
    hash: u32,
    openValues: ?*PObj.OUpValue,
    isDefault: bool,
    origin: ?*Module,
    sourceCode: ?[]u8,

    const Self = @This();

    pub fn init(self: *Self, gc: *Gc, name: []const u8) bool {
        self.name = gc.hal().alloc(u8, name.len) catch return false;
        @memcpy(self.name, name);
        return true;
    }

    pub fn new(gc: *Gc) ?*Module {
        const mod = gc.hal().create(Module) catch return null;

        mod.* = .{
            .origin = null,
            .globals = table.PankTable(){},
            .stdlibCount = 0,
            .stdProxies = undefined,
            .frameCount = 0,
            .name = undefined,
            .openValues = null,
            .isDefault = false,
            .sourceCode = null,
            .frames = undefined,
            .hash = 0,
        };
        mod.*.frames.stack[0] = undefined;
        mod.*.frames.count = 0;
        mod.*.stdProxies = std.ArrayListUnmanaged(StdLibProxy).initCapacity(
            gc.hal(),
            2,
        ) catch return null;
        return mod;
    }

    pub fn free(self: *Self, gc: *Gc) bool {
        gc.hal().free(self.name);
        self.globals.deinit(gc.hal());
        self.stdProxies.clearAndFree(gc.hal());
        self.stdProxies.deinit(gc.hal());
        gc.hal().destroy(self);
        return true;
    }

    fn markFrames(self: *Self, gc: *Gc) usize {
        var i: usize = 0;
        while (i < self.frameCount) : (i += 1) {
            gc.markObject(self.frames.stack[i].closure.parent());
        }
        return i;
    }
    fn markOpenUpvalues(self: *Self, gc: *Gc) i32 {
        var upv = self.openValues;
        var i: i32 = 0;

        while (upv) |u| {
            upv = u.next;
            gc.markObject(u.parent());
            i += 1;
        }

        return i;
    }
    pub fn mark(self: *Self, gc: *Gc) bool {
        _ = self.markFrames(gc);
        _ = self.markOpenUpvalues(gc);
        gc.markTable(self.globals);

        return true;
    }
};
const STDMAX: usize = 64;

pub const Gc = struct {
    internal_al: Allocator,
    al: Allocator,
    handyal: Allocator,
    objects: ?*PObj,
    strings: table.PankTable(),
    builtins: table.PankTable(),
    openUps: ?*PObj.OUpValue,
    alocAmount: usize,
    nextGc: usize,
    stack: ?*stck.VStack,
    compiler: ?*compiler.Compiler,
    grayStack: std.ArrayListUnmanaged(*PObj),
    pstdout: std.io.AnyWriter,
    pstderr: std.io.AnyWriter,
    modules: std.ArrayListUnmanaged(*Module),
    modCount: usize,
    stdlibs: [STDMAX]StdLibMod,
    stdlibCount: u32,
    stress: bool,
    debugGc: bool,
    debug: bool,
    noGc: bool,
    timestamp: u32,

    const Self = @This();

    pub fn new(al: Allocator, handlyal: Allocator) !*Gc {
        if (std.process.hasEnvVarConstant("PANKTI_STRESS") or flags.STRESS_GC) {
            STRESS_GC = true;
        }

        if (std.process.hasEnvVarConstant("PANKTI_DEBUG_GC") or flags.DEBUG_GC) {
            DEBUG_GC = true;
        }

        if (std.process.hasEnvVarConstant("PANKTI_DEBUG") or flags.DEBUG) {
            DEBUG = true;
        }

        if (std.process.hasEnvVarConstant("PANKTI_NO_GC") or flags.NO_GC) {
            NO_GC = true;
        }

        var timestamp: u32 = 0;

        if (utils.IS_WASM) {
            timestamp = @intCast(utils.getTimestamp());
        } else {
            timestamp = @intCast(std.time.timestamp());
        }

        const newgc = try al.create(Self);
        newgc.* = .{
            .internal_al = al,
            .al = undefined,
            .handyal = handlyal,
            .strings = table.PankTable(){},
            .builtins = table.PankTable(){},
            .objects = null,
            .openUps = null,
            .alocAmount = 0,
            .nextGc = 0,
            .stack = null,
            //.callstack = null,
            .compiler = null,
            .grayStack = std.ArrayListUnmanaged(*PObj){},
            .pstdout = undefined,
            .pstderr = undefined,
            .modules = undefined,
            .modCount = 0,
            .stdlibs = undefined,
            .stdlibCount = 0,
            .stress = STRESS_GC,
            .debugGc = DEBUG_GC,
            .debug = DEBUG,
            .noGc = NO_GC,
            .timestamp = @intCast(timestamp),
        };

        return newgc;
    }

    pub fn boot(
        self: *Self,
        stdout: anytype,
        stderr: anytype,
    ) void {
        self.al = self.allocator();
        self.pstdout = stdout;
        self.pstderr = stderr;
        self.modules = std.ArrayListUnmanaged(*Module){};
        self.stdlibs[0] = StdLibMod.new();
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
        const result = self.internal_al.rawResize(
            buf,
            bufalign,
            newlen,
            ret_addr,
        );

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

    pub inline fn hal(self: *Self) Allocator {
        return self.handyal;
    }

    pub fn newObj(
        self: *Self,
        otype: PObj.OType,
        comptime ParentType: type,
    ) !*ParentType {
        const ptr = try self.al.create(ParentType);
        ptr.parent().objtype = otype;
        if (DEBUG_GC) {
            ansicolors.TermColor('b', self.pstdout);

            try self.pstdout.print("[GC] (0x{x}) New Object: {s}", .{
                @intFromPtr(ptr),
                ptr.parent().objtype.toString(),
            });

            ansicolors.ResetColor(self.pstdout);
            try self.pstdout.print("\n", .{});
        }
        ptr.parent().isMarked = false;
        ptr.parent().next = self.objects;
        self.objects = ptr.parent();

        return ptr;
    }

    pub fn newString(self: *Self, chars: []u8, len: usize) !*PObj.OString {
        var ptr = try self.newObj(.Ot_String, PObj.OString);
        ptr.chars = chars;
        ptr.len = @intCast(len);
        //ptr.len = @truncate();
        ptr.obj.isMarked = true;
        ptr.hash = try utils.hashChars(chars, self);

        try self.strings.put(self.hal(), ptr, PValue.makeNil());
        ptr.obj.isMarked = false;

        return ptr;
    }

    pub fn newNative(self: *Self, v: *vm.Vm, n: PObj.ONativeFunction.NativeFn) ?PValue {
        const o = self.newObj(PObj.OType.Ot_NativeFunc, PObj.ONativeFunction) catch return null;
        v.stack.push(PValue.makeObj(o.parent())) catch return null;
        o.init(n);

        return v.stack.pop() catch return null;
    }

    pub fn copyString(self: *Gc, chars: []const u8, len: usize) !*PObj.OString {
        if (table.getString(
            self.strings,
            try utils.hashChars(chars, self),
            len,
        )) |interned| {
            if (DEBUG_GC) {
                dprint('b', self.pstdout, "[GC] Returning interned string : ", .{});
                _ = interned.print(self, null);

                dprint('n', self.pstdout, "\n", .{});
            }
            return interned;
        }

        const mem_chars = try self.getAlc().alloc(u8, len);
        @memcpy(mem_chars, chars);

        return self.newString(mem_chars, len);
    }

    pub fn copyStringU8(self: *Gc, chars: []const u8, len: usize) !*PObj.OString {
        _ = len;

        const result = try self.copyString(chars, chars.len);

        return result;
    }

    pub fn takeString(self: *Gc, chars: []const u8, len: u32) !*PObj.OString {
        if (self.strings.get(chars)) |interned| {
            try self.getAlc().free(chars);
            return interned;
        }

        return try self.newString(chars, len);
    }

    pub fn makeString(self: *Gc, chars: []const u8) PValue {
        const s = self.copyStringU8(chars, chars.len) catch {
            return PValue.makeNil();
        };
        return PValue.makeObj(s.parent());
    }

    pub fn printTable(self: *Self, tab: *table.PankTable(), tabname: []const u8) void {
        var ite = tab.iterator();

        self.pstdout.print("==== {s} ====\n", .{tabname}) catch return;
        while (ite.next()) |value| {
            self.pstdout.print("[", .{}) catch return;
            _ = value.key_ptr.*.print(self);
            self.pstdout.print("] -> [", .{}) catch return;
            _ = value.value_ptr.*.printVal(self);
            self.pstdout.print("]\n", .{}) catch return;
        }
        self.pstdout.print("==============\n", .{}) catch return;
    }

    pub fn freeSingleObject(self: *Self, obj: *PObj) void {
        //if (false) {
        //    ansicolors.TermColor('p', self.pstdout);
        //    self.pstdout.print("[GC] (0x{x}) Free Object: {s} : [ ", .{
        //        @intFromPtr(obj),
        //        obj.objtype.toString(),
        //    }) catch return;
        //    _ = obj.printObj(self);
        //    self.pstdout.print(" ]\n", .{}) catch return;
        //    ansicolors.ResetColor(self.pstdout);
        //}
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

            .Ot_Hmap => {
                obj.asHmap().free(self);
            },

            .Ot_Error => {
                obj.asOErr().free(self);
            },

            .Ot_Module => {
                obj.asMod().free(self);
            },

            .Ot_BigInt => {
                const bobj = obj.child(PObj.OBigInt);
                bobj.free(self);
            },
        }

        return;
    }

    pub fn freeObjects(self: *Self) void {
        var object: ?*PObj = self.objects;
        if (DEBUG_GC) {
            var tempObj = self.objects;

            while (tempObj) |obj| {
                ansicolors.TermColor('p', self.pstdout);
                self.pstdout.print("[GC] (0x{x}) Free Object: {s} : [ ", .{
                    @intFromPtr(obj),
                    obj.objtype.toString(),
                }) catch return;
                _ = obj.printObj(self, null);
                self.pstdout.print(" ]\n", .{}) catch return;
                ansicolors.ResetColor(self.pstdout);
                tempObj = obj.next;
            }
        }

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
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "-> FINISHING GC <-\n", .{});
            //self.pstdout.print("TOTAL BYTES ALLOCATED-> {d}bytes\n" , .{self.alocAmount});
        }
        self.freeObjects();
        self.strings.deinit(self.hal());
        self.builtins.deinit(self.hal());
        self.grayStack.deinit(self.hal());

        var i: usize = 0;

        while (i < self.modules.items.len) : (i += 1) {
            _ = self.modules.items[i].free(self);
        }

        i = 0;
        while (i < self.stdlibCount) : (i += 1) {
            self.stdlibs[i].free(self);
        }

        self.modules.deinit(self.hal());

        if (DEBUG_GC) {
            dprint('r', self.pstdout, "<- TERMINATE GC ->\n", .{});
            //self.pstdout.print("TOTAL BYTES ALLOCATED-> {d}bytes\n" , .{self.alocAmount});
        }
    }

    pub fn tryCollect(self: *Self) void {
        if (NO_GC) return;
        //std.debug.print("STRESS->{any}\n", .{self.stress});
        if ((self.alocAmount > self.nextGc) or STRESS_GC) {
            self.collect();
        }
    }

    pub fn collect(self: *Self) void {
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Marking Roots\n", .{});
        }
        self.markRoots();

        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Marking Roots\n", .{});
            dprint('r', self.pstdout, "[GC] Tracing Refs\n", .{});
        }
        self.traceRefs();

        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Tracing Refs\n", .{});

            dprint('r', self.pstdout, "[GC] Cleaning Strings\n", .{});
        }
        self.removeTableUnpainted(&self.strings);
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Cleaning Strings\n", .{});

            dprint('r', self.pstdout, "[GC] Sweeping\n", .{});
        }
        self.sweep();
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Sweeping\n", .{});
        }
    }

    fn removeTableUnpainted(self: *Self, tab: *table.PankTable()) void {
        var ite = tab.keyIterator();

        while (ite.next()) |key| {
            if (!key.*.parent().isMarked) {
                if (DEBUG_GC) {
                    dprint('p', self.pstdout, "[GC] Removing String ", .{});
                    _ = key.*.print(self, null);
                    dprint('p', self.pstdout, "\n", .{});
                }
                _ = tab.removeByPtr(key);
            }
        }
    }

    fn markModules(self: *Self) void {
        var i: usize = 0;
        while (i < self.modules.items.len) : (i += 1) {
            _ = self.modules.items[i].mark(self);
        }
    }
    fn markRoots(self: *Self) void {
        if (self.stack) |stack| {
            if (DEBUG_GC) {
                dprint('r', self.pstdout, "[GC] Marking Stack \n", .{});
            }
            for (0..@intCast(stack.presentcount())) |i| {
                self.markValue(stack.stack[i]);
            }
            if (DEBUG_GC) {
                dprint('r', self.pstdout, "[GC] Finished Marking Stack \n", .{});
            }
        }

        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Marking Builtins \n", .{});

            dprint('r', self.pstdout, "[GC] Finished Marking Builtins \n", .{});
        }

        self.markTable(self.builtins);
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Marking Modules \n", .{});
        }
        self.markModules();
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Marking Modules \n", .{});

            dprint('r', self.pstdout, "[GC] Marking Compiler Roots \n", .{});
        }
        self.markCompilerRoots();
        if (DEBUG_GC) {
            dprint('r', self.pstdout, "[GC] Finished Marking Compiler Roots \n", .{});
        }

        var i: usize = 0;

        while (i < self.stdlibCount) : (i += 1) {
            self.markTable(self.stdlibs[i].items);
        }
    }

    fn markTable(self: *Self, tab: table.PankTable()) void {
        var ite = tab.iterator();

        while (ite.next()) |val| {
            self.markObject(val.key_ptr.*.parent());
            self.markValue(val.value_ptr.*);
        }
    }

    pub fn markValue(self: *Self, v: PValue) void {
        if (v.isObj()) {
            self.markObject(v.asObj());
        }
    }

    pub fn markObject(self: *Self, obj: ?*PObj) void {
        if (obj) |o| {
            if (o.isMarked) {
                return;
            }

            if (DEBUG_GC) {
                dprint('g', self.pstdout, "[GC] Marking Object : {s} : [ ", .{
                    o.getType().toString(),
                });
                _ = o.printObj(self, null);

                dprint('g', self.pstdout, " ] \n", .{});
            }
            o.isMarked = true;
            self.grayStack.append(self.hal(), o) catch return;
        }
    }

    fn traceRefs(self: *Self) void {
        while (self.grayStack.items.len > 0) {
            const obj = self.grayStack.pop();
            self.paintObject(obj);
        }
    }

    fn paintObject(self: *Self, obj: *PObj) void {
        //std.debug.print("self -> {any}\n" , .{obj.objtype});
        switch (obj.getType()) {
            .Ot_String, .Ot_NativeFunc, .Ot_Error => {},
            .Ot_BigInt => {},
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

                var i: usize = 0;
                while (i < arr.values.items.len) : (i += 1) {
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
                var i: usize = 0;
                while (i < c.upc) : (i += 1) {
                    self.markObject(c.upvalues[i].parent());
                }
            },

            .Ot_Hmap => {
                const hm = obj.asHmap();

                var ite = hm.values.iterator();

                while (ite.next()) |pair| {
                    self.markValue(pair.key_ptr.*);
                    self.markValue(pair.value_ptr.*);
                }
            },

            .Ot_Module => {
                self.markObject(obj.asMod().name.parent());
            },
            //else => {
            //    return;
            //}
        }
    }

    fn sweep(self: *Self) void {
        var previous: ?*PObj = null;
        var object = self.objects;

        if (DEBUG_GC) {
            var tempObj = self.objects;

            while (tempObj) |obj| {
                if (obj.isMarked) {
                    tempObj = obj.next;
                } else {
                    ansicolors.TermColor('p', self.pstdout);
                    self.pstdout.print("[GC] (0x{x}) Free Object: {s} : [ ", .{
                        @intFromPtr(obj),
                        obj.objtype.toString(),
                    }) catch return;
                    _ = obj.printObj(self, null);
                    self.pstdout.print(" ]\n", .{}) catch return;
                    ansicolors.ResetColor(self.pstdout);
                    tempObj = obj.next;
                }
            }
        }

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
