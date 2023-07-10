//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const ins = @import("instruction.zig");
const OpCode = @import("instruction.zig").OpCode;
const comp = @import("compiler.zig");
const Compiler = comp.Compiler;
const gcz = @import("gc.zig");
const Gc = @import("gc.zig").Gc;
const vl = @import("value.zig");
const PValue = vl.PValue;
const PObj = @import("object.zig").PObj;
const utils = @import("utils.zig");
const table = @import("table.zig");
const Allocator = std.mem.Allocator;
const flags = @import("flags.zig");
const builtins = @import("builtins.zig");
const kws = @import("lexer/keywords.zig");
const _stack = @import("stack.zig");
const VStack = _stack.VStack;
const CallStack = _stack.CallStack;
const CallFrame = _stack.CallFrame;
const openfile = @import("openfile.zig");
const stdlib = @import("stdlib/stdlib.zig");

pub const IntrpResult = enum(u8) {
    Ok,
    CompileError,
    RuntimeError,

    pub fn toString(self: IntrpResult) []const u8 {
        switch (self) {
            .Ok => {
                return "Ok";
            },
            .CompileError => {
                return "CompileError";
            },
            .RuntimeError => {
                return "RuntimeError";
            },
        }
    }
};

pub const Vm = struct {
    compiler: *Compiler,
    stack: VStack,
    gc: *Gc,
    cmod: *gcz.Module,

    const Self = @This();

    pub fn newVm(al: Allocator) !*Vm {
        const v = try al.create(Vm);
        return v;
    }

    pub fn bootVm(self: *Self, gc: *Gc) void {
        self.*.gc = gc;
        self.*.compiler = undefined;
        const mod = gcz.Module.new(gc).?;
        if (!mod.init(self.gc, &[_]u32{ '_', 'd', '_' })) return;
        self.*.gc.modules.append(self.gc.hal(), mod) catch return;
        self.*.gc.modCount += 1;

        self.cmod = self.*.gc.modules.items[0];

        self.*.stack.stack[0] = PValue.makeNil();

        self.*.stack.count = 0;
        self.*.stack.top = self.stack.stack[0..];
        self.*.stack.head = self.stack.stack[0..];

        self.gc.stack = &self.stack;
        self.defineNative(
            &[_]u32{ 'c', 'l', 'o', 'c', 'k' },
            builtins.nClock,
        ) catch return;

        self.defineNative(&[_]u32{ 'l', 'e', 'n' }, builtins.nLen) catch return;
        self.defineNative(&kws.K_EN_SHOW, builtins.nShow) catch return;
        self.defineNative(&kws.K_BN_SHOW, builtins.nBnShow) catch return;

        //self.defineNative(
        //    &kws.K_PN_SHOW,
        //    builtins.nShow,
        //) catch return;

        //self.gc.callstack = &self.callframes;

        //self.*.ip = self.*.ins.code.items.ptr;

        //self.strings = table.StringTable(){};

    }

    pub fn interpretRaw(self: *Self, inst: *ins.Instruction) IntrpResult {
        _ = inst;
        _ = self;
        //@memcpy(self.ins.*, inst.);
        //self.ip = inst.code.items.ptr;
        //self.ins = inst.*;
        //return self.run();
    }

    pub fn interpret(self: *Self, source: []u32) IntrpResult {
        self.compiler = Compiler.new(
            source,
            self.gc,
            comp.FnType.Ft_SCRIPT,
        ) catch return .RuntimeError;

        self.gc.compiler = self.compiler;

        const rfunc: ?*PObj.OFunction = self.compiler.compile(
            source,
        ) catch return .CompileError;

        if (rfunc) |f| {
            self.stack.push(f.parent().asValue()) catch {
                self.throwRuntimeError("failed to push function to stack", .{});
                return .RuntimeError;
            };
            //f.name = self.gc.copyString(&[_]u32{'_' , 'd' , '_'}, 3) catch return .RuntimeError;
            const cls = PObj.OClosure.new(self.gc, f) catch {
                self.throwRuntimeError("failed to create a closure", .{});
                return .RuntimeError;
            };
            cls.globOwner = 0;
            _ = self.stack.pop() catch return .RuntimeError;
            _ = self.stack.push(
                cls.parent().asValue(),
            ) catch return .RuntimeError;

            self.*.gc.modules.items[0].frames.stack[0] = .{
                .closure = cls,
                .ip = f.ins.code.items.ptr,
                .slots = self.stack.stack[0..],
                .globOwner = 0,
                .globals = &self.*.gc.modules.items[0].globals,
            };

            self.*.gc.modules.items[0].frames.count = 1;
            //self.gc.callstack = &self.callframes;

            cls.globals = &self.*.gc.modules.items[0].globals;
            self.cmod = self.*.gc.modules.items[0];
            self.cmod.frameCount = 1;
            self.cmod.frames.count = 1;
            self.cmod.isDefault = true;

            return self.run();
        } else {
            return .RuntimeError;
        }
    }

    pub fn freeVm(self: *Self, al: Allocator) void {
        self.compiler.free(self.gc.getAlc());
        self.gc.free();
        self.resetStack();
        al.destroy(self);
    }

    fn resetStack(self: *Self) void {
        //self.stackTop = 0;
        self.stack.top = self.stack.stack[0..];
        self.stack.count = 0;
    }

    pub fn peek(self: *Self, dist: usize) PValue {
        return (self.stack.top - 1 - dist)[0];
    }

    fn throwRuntimeError(
        self: *Self,
        comptime msg: []const u8,
        args: anytype,
    ) void {
        const frame = self.cmod.frames.stack[self.cmod.frameCount - 1];
        //const frame = &self.callframes.stack[self.callframes.count - 1];
        const i = @intFromPtr(frame.ip) - @intFromPtr(
            frame.closure.function.ins.code.items.ptr,
        ) - 1;
        self.gc.pstdout.print("\nRuntime Error Occured in line {}\n", .{
            frame.closure.function.ins.pos.items[i].line,
        }) catch return;
        self.gc.pstdout.print(msg, args) catch return;
        self.gc.pstdout.print("\n", .{}) catch return;

        var j: i64 = @intCast(self.cmod.frameCount - 1);
        while (j >= 0) {
            const f = &self.cmod.frames.stack[@intCast(j)];
            const fun = f.closure.function;
            const instr = @intFromPtr(f.ip) - @intFromPtr(
                fun.ins.code.items.ptr,
            ) - 1;
            self.gc.pstdout.print("[line {d}] in ", .{
                fun.ins.pos.items[instr].line,
            }) catch return;
            if (fun.name) |n| {
                utils.printu32(n.chars, self.gc.pstdout);
                self.gc.pstdout.print("()\n", .{}) catch return;
            } else {
                self.gc.pstdout.print("<script>\n", .{}) catch return;
            }

            j -= 1;
        }

        self.resetStack();
    }

    pub fn debugStack(self: *Self) void {
        self.gc.pstdout.print("==== STACK ====\n", .{}) catch return;
        if (self.stack.presentcount() > 0) {
            const presentcount = self.stack.presentcount();
            for (0..presentcount) |i| {
                const val = self.stack.stack[i];
                self.gc.pstdout.print("[ |{:0>2}| {s:>4}", .{
                    presentcount - 1 - i,
                    " ",
                }) catch return;
                val.printVal(self.gc);
                self.gc.pstdout.print(" ]\n", .{}) catch return;
            }
        }
        self.gc.pstdout.print("===============\n\n", .{}) catch return;
    }

    pub fn defineNative(
        self: *Self,
        name: []const u32,
        func: PObj.ONativeFunction.NativeFn,
    ) !void {
        const nstr = try self.gc.copyString(name, @intCast(name.len));

        try self.stack.push(nstr.parent().asValue());
        //self.debugStack();

        var nf = try self.gc.newObj(.Ot_NativeFunc, PObj.ONativeFunction);

        nf.init(func);
        try self.stack.push(nf.parent().asValue());

        //nf.print();
        try self.gc.builtins.put(
            self.gc.hal(),
            self.stack.stack[0].asObj().asString(),
            self.stack.stack[1],
        );

        _ = try self.stack.pop();
        _ = try self.stack.pop();
    }

    fn doBinaryOpAdd(self: *Self) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(
                PValue.makeNumber(a.asNumber() + b.asNumber()),
            ) catch return false;
            return true;
        } else if (a.isString() and b.isString()) {
            const bs = b.asObj().asString();
            const as = a.asObj().asString();

            var temp_chars = self.gc.hal().alloc(
                u32,
                as.chars.len + bs.chars.len,
            ) catch return false;
            var i: usize = 0;

            while (i < as.chars.len) {
                temp_chars[i] = as.chars[i];
                i += 1;
            }

            while (i - as.chars.len < bs.chars.len) {
                temp_chars[i] = bs.chars[i - as.chars.len];
                i += 1;
            }

            const s = self.gc.copyString(
                temp_chars,
                @intCast(temp_chars.len),
            ) catch {
                self.gc.hal().free(temp_chars);
                return false;
            };
            self.stack.push(s.obj.asValue()) catch {
                self.gc.hal().free(temp_chars);
                return false;
            };
            self.gc.hal().free(temp_chars);
            //self.gc.printTable(&self.gc.strings , "STRINGS");
            return true;
        } else {
            return false;
        }
    }

    fn doBinaryOpSub(self: *Self) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(
                PValue.makeNumber(a.asNumber() - b.asNumber()),
            ) catch return false;
            return true;
        } else {
            return false;
        }
    }

    fn doBinaryOpMul(self: *Self) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(
                PValue.makeNumber(a.asNumber() * b.asNumber()),
            ) catch return false;
            return true;
        } else {
            return false;
        }
    }

    fn doBinaryOpPow(self: *Self) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(
                PValue.makeNumber(
                    std.math.pow(f64, a.asNumber(), b.asNumber()),
                ),
            ) catch return false;
            return true;
        } else {
            return false;
        }
    }

    fn doBinaryOpDiv(self: *Self) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(
                PValue.makeNumber(a.asNumber() / b.asNumber()),
            ) catch return false;
            return true;
        } else {
            return false;
        }
    }

    const BinaryComp = enum(u8) {
        C_Gt,
        C_Lt,
        C_Gte,
        C_Lte,
    };

    fn doBinaryOpComp(self: *Self, op: BinaryComp) bool {
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            var val: bool = false;
            const av = a.asNumber();
            const bv = b.asNumber();
            switch (op) {
                .C_Gt => val = av > bv,
                .C_Lt => val = av < bv,
                .C_Gte => val = av >= bv,
                .C_Lte => val = av <= bv,
            }
            self.stack.push(PValue.makeBool(val)) catch return false;
            return true;
        } else {
            return false;
        }
    }

    const ValueComp = enum(u8) {
        C_Eq,
        C_Neq,
    };

    fn doValueComp(self: *Self, op: ValueComp) bool {
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;
        var val: bool = false;
        switch (op) {
            .C_Eq => {
                val = a.isEqual(b);
            },
            .C_Neq => {
                val = !a.isEqual(b);
            },
        }

        self.stack.push(PValue.makeBool(val)) catch return false;

        return true;
    }

    fn call(self: *Self, closure: *PObj.OClosure, argc: u8) bool {
        if (argc != closure.function.arity) {
            self.throwRuntimeError("Function Expected Arg != Got", .{});
            return false;
        }

        var frame = &self.cmod.frames.stack[self.cmod.frameCount];
        self.cmod.frames.count += 1;
        self.cmod.frameCount += 1;
        frame.closure = closure;
        frame.ip = closure.function.ins.code.items.ptr;
        frame.slots = self.stack.top - argc - 1;
        frame.globOwner = closure.globOwner;
        if (closure.globals) |g| {
            frame.globals = g;
        }
        return true;
    }

    fn callVal(self: *Self, calle: PValue, argc: u8) bool {
        if (calle.isObj()) {
            switch (calle.asObj().objtype) {
                .Ot_Closure => {
                    return self.call(calle.asObj().asClosure(), argc);
                },
                .Ot_NativeFunc => {
                    const f: *PObj.ONativeFunction =
                        calle.asObj().asNativeFun();

                    const result = f.func(
                        self.gc,
                        argc,
                        (self.stack.top - argc)[0..argc],
                    );
                    self.stack.top -= argc + 1;

                    if (result.isError()) {
                        const eo: *PObj.OError = result.asObj().asOErr();
                        _ = eo.print(self.gc);
                        return false;
                    }

                    self.stack.push(result) catch return false;
                    return true;
                },
                else => {},
            }
        }

        _ = calle.printVal(self.gc);
        self.throwRuntimeError("Can only call functions", .{});
        return false;
    }

    fn captureUpval(self: *Self, local: *PValue) !*PObj.OUpValue {
        var prevup: ?*PObj.OUpValue = null;
        var upval = self.gc.openUps;

        while (upval != null and @intFromPtr(upval.?.location) > @intFromPtr(
            local,
        )) {
            prevup = upval;
            upval = upval.?.next;
        }

        if (upval != null and @intFromPtr(upval.?.location) == @intFromPtr(
            local,
        )) {
            return upval.?;
        }

        const newUp = try self.gc.newObj(.Ot_UpValue, PObj.OUpValue);
        newUp.init(local);
        newUp.next = upval;

        if (prevup == null) {
            self.gc.openUps = newUp;
        } else {
            prevup.?.next = newUp;
        }

        return newUp;
    }

    fn closeUpv(self: *Self, last: [*]PValue) void {
        while (self.gc.openUps) |oup| {
            if (!(@intFromPtr(oup) >= @intFromPtr(last))) {
                break;
            }

            var upv = oup;
            upv.closed = oup.location.*;
            upv.location = &upv.closed;
            self.gc.openUps = upv.next;
        }
    }

    fn compileModule(self: *Self, rawSource: []const u8) (?*PObj.OFunction) {
        const source = utils.u8tou32(
            rawSource,
            self.gc.hal(),
        ) catch return null;

        const modComp = Compiler.new(
            source,
            self.gc,
            .Ft_SCRIPT,
        ) catch return null;

        self.gc.compiler = modComp;

        const f = modComp.compileModule(source) catch return null;
        modComp.markRoots(self.gc);
        self.compiler.markRoots(self.gc);

        if (f) |ofunu| {
            ofunu.ins.disasm("import");

            self.stack.push(PValue.makeObj(ofunu.parent())) catch {
                self.gc.hal().free(source);
                return null;
            };
        }

        self.gc.hal().free(source);

        //modComp.free(self.gc.getAlc());

        return f;
    }

    pub fn pushStdlib(self: *Self, importName: []const u32) bool {
        if (utils.matchU32(stdlib.OsName, importName)) {
            stdlib.pushStdlibOs(self);
            return true;
        } else if (utils.matchU32(stdlib.BnOsName, importName)) {
            stdlib.pushStdlibBnOs(self);
            return true;
        }

        return false;
    }

    fn importStdlib(self: *Self, customName: []const u32, importName: []const u32) bool {
        const strname = self.gc.copyString(customName, @intCast(customName.len)) catch {
            self.throwRuntimeError("failed to convert import name to string", .{});
            return false;
        };

        self.stack.push(PValue.makeObj(strname.parent())) catch {
            self.throwRuntimeError("Failed to push import name to stack", .{});
            return false;
        };

        const objmod = PObj.OModule.new(self, self.gc, customName) orelse {
            self.throwRuntimeError("Failed to create a new module object", .{});
            return false;
        };

        self.stack.push(PValue.makeObj(objmod.parent())) catch {
            self.throwRuntimeError("failed to push module object to stack", .{});
            return false;
        };

        self.cmod.globals.put(
            self.gc.hal(),
            self.peek(1).asObj().asString(),
            self.peek(0),
        ) catch {
            self.throwRuntimeError("failed to push module object to global table", .{});
            return false;
        };

        //std.debug.print("\n\nNew Module -> C{any}|H{d}\n\n", .{objmod.name.chars , objmod.name.hash} );
        var newProxy = gcz.StdLibProxy.new(objmod.name.hash, objmod.name.chars);

        if (self.gc.stdlibCount < 1) {
            if (!self.pushStdlib(importName)) {
                self.throwRuntimeError("No such stdlib module found", .{});
                return false;
            }
            self.gc.stdlibs[0].owners.append(self.gc.hal(), self.cmod.hash) catch {
                self.throwRuntimeError("failed to push owners", .{});
                return false;
            };
            self.gc.stdlibs[0].ownerCount += 1;
            //newProxy.proxyName =
            newProxy.originName = self.gc.stdlibs[0].name;
            newProxy.stdmod = &self.gc.stdlibs[0];
            self.cmod.stdProxies.append(self.gc.hal(), newProxy) catch {
                self.throwRuntimeError("failed to add stdlib proxy to current module", .{});
                return false;
            };
            self.cmod.stdlibCount += 1;

            //std.debug.print("\n\nProxy -> {any}{d}\n\n" , .{self.gc.stdlibs[0].owners , self.cmod.hash});

        }

        _ = self.stack.pop() catch return false;
        _ = self.stack.pop() catch return false;

        return true;
    }

    fn importModule(self: *Self, customName: []const u32, importName: []const u32) bool {
        if (stdlib.IsStdlib(importName)) {
            return self.importStdlib(customName, importName);
        }

        if (utils.IS_WASM) {
            self.throwRuntimeError("File module import in wasm is unsupported", .{});
            return false;
        }
        const filename = utils.u32tou8(importName, self.gc.hal()) catch {
            self.gc.pstdout.print(
                "failed to convert filename",
                .{},
            ) catch return false;
            return false;
        };
        const src = openfile.openfile(filename, self.gc.hal()) catch {
            self.gc.hal().free(filename);
            return false;
        };

        self.gc.hal().free(filename);

        const newModule: *gcz.Module = gcz.Module.new(self.gc) orelse {
            self.throwRuntimeError("Failed to create a new module", .{});
            return false;
        };
        if (!newModule.init(self.gc, customName)) {
            self.throwRuntimeError("Failed to init newly created module", .{});
            return false;
        }
        newModule.origin = self.cmod;
        newModule.isDefault = false;

        self.gc.modules.append(self.gc.hal(), newModule) catch {
            self.throwRuntimeError("failed to add newly created module to module table", .{});
            return false;
        };
        self.gc.modCount += 1;

        const objmod = PObj.OModule.new(self, self.gc, customName) orelse {
            self.throwRuntimeError("failed to create new module object", .{});
            return false;
        };
        self.stack.push(PValue.makeObj(objmod.parent())) catch {
            self.throwRuntimeError("failed to add newly created module object to stack", .{});
            return false;
        };
        self.gc.modules.items[self.gc.modCount - 1].hash = objmod.name.hash;

        const objString = self.gc.copyString(
            customName,
            @intCast(customName.len),
        ) catch {
            self.throwRuntimeError("failed to create a new string for object module", .{});
            return false;
        };
        self.stack.push(PValue.makeObj(objString.parent())) catch {
            self.throwRuntimeError("failed to add newly created string to stack", .{});
            return false;
        };

        self.cmod.globals.put(
            self.gc.hal(),
            self.peek(0).asObj().asString(),
            self.peek(1),
        ) catch {
            self.throwRuntimeError("failed to add new module object to global table", .{});
            return false;
        };

        const source = utils.u8tou32(src, self.gc.hal()) catch {
            self.throwRuntimeError("failed to encode module source to UTF-32", .{});
            return false;
        };
        const modComp = Compiler.new(source, self.gc, .Ft_SCRIPT) catch {
            self.throwRuntimeError("failed to create a new compiler for module", .{});
            return false;
        };
        self.gc.compiler = modComp;
        const rawFunc = modComp.compileModule(source) catch {
            self.throwRuntimeError("failed to compile module source code", .{});
            return false;
        };

        modComp.markRoots(self.gc);
        self.compiler.markRoots(self.gc);

        if (rawFunc) |ofunu| {
            ofunu.ins.disasm("import");

            self.stack.push(PValue.makeObj(ofunu.parent())) catch {
                self.gc.hal().free(source);
                self.throwRuntimeError("failed to push new source function to stack", .{});
                return false;
            };
        } else {
            self.throwRuntimeError("compiler error has occured in compiling the module", .{});
            return false;
        }

        self.gc.hal().free(source);
        const f = rawFunc.?;

        f.fromMod = true;

        const cls = self.gc.newObj(.Ot_Closure, PObj.OClosure) catch {
            self.throwRuntimeError("failed to create new closure object", .{});
            return false;
        };

        cls.parent().isMarked = true;
        cls.init(self.gc, f) catch {
            self.throwRuntimeError("failed to initialize new closure object", .{});
            return false;
        };
        self.stack.push(PValue.makeObj(cls.parent())) catch {
            self.gc.hal().free(src);
            self.throwRuntimeError("failed to push newly created closure object to stack", .{});
            return false;
        };

        cls.globals = &newModule.globals;
        cls.globOwner = newModule.hash;

        _ = self.stack.pop() catch return false; //closure
        _ = self.stack.pop() catch return false; //function
        _ = self.stack.pop() catch return false; //objmod
        self.gc.hal().free(src);

        modComp.markRoots(self.gc);
        self.compiler.markRoots(self.gc);
        //self.compiler = modComp;
        self.cmod = newModule;
        return self.call(cls, 0);
    }

    fn getModuleByHash(self: *Self, hash: u32) ?*gcz.Module {
        var i: usize = 0;

        while (i < self.gc.modules.items.len) : (i += 1) {
            const mod = self.gc.modules.items[i];
            if (mod.hash == hash) {
                return mod;
            }
        }

        return null;
    }

    fn getModProxy(self: *Self, name: u32, mod: *gcz.Module) u32 {
        _ = self;
        var i: usize = 0;

        while (i < mod.stdlibCount) : (i += 1) {
            const proxy = mod.stdProxies.items[i];

            if (proxy.proxyHash == name) {
                return proxy.stdmod.hash;
            }
        }

        return 0;
    }

    fn getStdlibByHash(self: *Self, hash: u32, mod: *gcz.Module) ?*gcz.StdLibMod {
        const modHash = mod.hash;
        var i: usize = 0;

        while (i < self.gc.stdlibCount) : (i += 1) {
            const m = &self.gc.stdlibs[i];

            if (m.hash == hash) {
                var j: usize = 0;
                while (j < m.ownerCount) : (j += 1) {
                    if (m.owners.items[j] == modHash) {
                        return m;
                    }
                }
            }
        }

        return null;
    }

    fn run(self: *Self) IntrpResult {
        //std.debug.print("{any}\n" , .{self.cmod});
        var frame: *CallFrame =
            &self.cmod.frames.stack[self.cmod.frameCount - 1];
        //&self.callframes.stack[self.callframes.count - 1];

        while (true) {
            if (flags.DEBUG and flags.DEBUG_GLOBS) {
                self.gc.printTable(&self.gc.globals, "GLOBS");
                self.gc.printTable(&self.gc.strings, "STRINGS");
            }

            if (flags.DEBUG and flags.STRESS_GC) {
                self.gc.tryCollect();
            }
            const op = frame.readByte();

            if (flags.DEBUG) {
                //self.gc.pstdout.print("Op -> {s}\n", .{op.toString()}) catch return .RuntimeError;
                //self.debugStack();
            }

            switch (op) {
                .Op_GetModProp => {
                    const rawMod = self.peek(0);
                    if (!rawMod.isMod()) {
                        self.throwRuntimeError(
                            "Dot field can be accessed used on modules",
                            .{},
                        );
                        return .RuntimeError;
                    }
                    const modObj = rawMod.asObj().asMod();
                    const prop = frame.readStringConst();
                    //_ = self.stack.pop() catch return .RuntimeError;
                    const rawModule = self.getModuleByHash(modObj.name.hash);

                    if (rawModule) |module| {
                        if (module.globals.get(prop)) |value| {
                            _ = self.stack.pop() catch {
                                self.throwRuntimeError("failed to pop", .{});
                                return .RuntimeError;
                            };

                            self.stack.push(value) catch {
                                self.throwRuntimeError("failed to push value", .{});
                                return .RuntimeError;
                            };
                        } else {
                            self.throwRuntimeError(
                                "Undefined module variable",
                                .{},
                            );
                            return .RuntimeError;
                        }
                    } else {
                        const mhash = self.getModProxy(modObj.name.hash, self.cmod);
                        //std.debug.print("\n\nmhash->{d}\n\n" , .{mhash});
                        //std.debug.print("\n\n->{any}<-\n\n" , .{ self.cmod.stdProxies.items[0].proxyHash });

                        if (mhash != 0) {
                            //std.debug.print("\n->{}<-\n", .{self.gc.stdlibs[0].hash});
                            const rawSmod = self.getStdlibByHash(mhash, self.cmod);
                            if (rawSmod) |smod| {
                                if (smod.items.get(prop)) |value| {
                                    _ = self.stack.pop() catch {
                                        self.throwRuntimeError("failed to pop", .{});
                                        return .RuntimeError;
                                    };
                                    //_ = value.printVal(self.gc);

                                    self.stack.push(value) catch {
                                        self.throwRuntimeError("failed to push value", .{});
                                        return .RuntimeError;
                                    };
                                } else {
                                    self.throwRuntimeError(
                                        "Undefined stlib module variable",
                                        .{},
                                    );
                                    return .RuntimeError;
                                }
                            } else {
                                self.throwRuntimeError("module not found", .{});
                                return .RuntimeError;
                            }
                        } else {
                            self.throwRuntimeError("module not found", .{});
                            return .RuntimeError;
                        }
                        //std.debug.print("MODULE -> {any}\n" , .{module});
                        //_ = rawMod.printVal(self.gc);
                    }
                },
                .Op_Import => {
                    const rawCustomName = frame.readConst();
                    if (!rawCustomName.isString()) {
                        self.throwRuntimeError(
                            "Import name must be a identifier",
                            .{},
                        );
                        return .RuntimeError;
                    }
                    const rawFileName = self.peek(0);

                    if (!rawFileName.isString()) {
                        self.throwRuntimeError(
                            "import filename must be a string",
                            .{},
                        );
                        return .RuntimeError;
                    }

                    if (!self.importModule(
                        rawCustomName.asObj().asString().chars,
                        rawFileName.asObj().asString().chars,
                    )) {
                        self.throwRuntimeError("Failed to import module", .{});
                        return .RuntimeError;
                    }

                    frame = &self.cmod.frames.stack[self.cmod.frames.count - 1];
                },
                .Op_Hmap => {
                    const count = frame.readU16();
                    const mapObj = self.gc.newObj(.Ot_Hmap, PObj.OHmap) catch {
                        self.throwRuntimeError(
                            "Failed to create a new map object",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    mapObj.init(self.gc);
                    self.stack.push(PValue.makeObj(mapObj.parent())) catch {
                        self.throwRuntimeError(
                            "Failed to push new obj to stack",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    var i: usize = count * 2;

                    while (i > 0) : (i -= 2) {
                        if (!mapObj.addPair(
                            self.gc,
                            self.peek(i),
                            self.peek(i - 1),
                        )) {
                            self.throwRuntimeError(
                                "Failed to add pair to map",
                                .{},
                            );
                            return .RuntimeError;
                        }
                    }

                    self.stack.top -= count * 2 + 1;
                    self.stack.push(PValue.makeObj(mapObj.parent())) catch {
                        self.throwRuntimeError(
                            "Failed to push new obj to stack",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },
                .Op_SubAssign => {
                    var rawObj = self.peek(2);
                    var rawIndex = self.peek(1);
                    var newObj = self.peek(0); // New object;

                    if (!rawIndex.isNumber()) {
                        self.throwRuntimeError(
                            "Index must be a number",
                            .{},
                        );
                        return .RuntimeError;
                    }

                    const rawIndexNum = rawIndex.asNumber();

                    if (rawIndexNum < 0 or @ceil(rawIndexNum) != rawIndexNum) {
                        self.throwRuntimeError(
                            "index must be non negetive integer",
                            .{},
                        );
                        return .RuntimeError;
                    }

                    const index: usize = @intFromFloat(rawIndexNum);

                    if (rawObj.isObj() and rawObj.asObj().isArray()) {
                        if (rawObj.asObj().isArray()) {
                            const arr = rawObj.asObj().asArray();

                            if (index >= arr.count) {
                                self.throwRuntimeError(
                                    "Index out of range",
                                    .{},
                                );
                                return .RuntimeError;
                            }
                            arr.values.replaceRange(
                                self.gc.hal(),
                                index,
                                1,
                                &[1]PValue{newObj},
                            ) catch {
                                self.throwRuntimeError(
                                    "failed to replace value",
                                    .{},
                                );
                                return .RuntimeError;
                            };

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError("failed to pop", .{});
                                return .RuntimeError;
                            };

                            self.stack.push(PValue.makeNil()) catch {
                                self.throwRuntimeError(
                                    "failed to push",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                        }
                    } else {
                        self.throwRuntimeError(
                            "Subscript assignment only works on arrays",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },
                .Op_Index => {
                    var rawIndex = self.peek(0);
                    var rawObj = self.peek(1);

                    if (rawObj.isObj() and
                        (rawObj.asObj().isString() or
                        rawObj.asObj().isArray()))
                    {
                        if (!rawIndex.isNumber()) {
                            self.throwRuntimeError(
                                "Index must be number",
                                .{},
                            );
                            return .RuntimeError;
                        }

                        const rawIndexNum = rawIndex.asNumber();

                        if (rawIndexNum < 0 or
                            @ceil(rawIndexNum) != rawIndexNum)
                        {
                            self.throwRuntimeError(
                                "Index must be non negetive integer",
                                .{},
                            );
                            return .RuntimeError;
                        }

                        const index: usize = @intFromFloat(rawIndexNum);

                        if (rawObj.asObj().isArray()) {
                            const arr = rawObj.asObj().asArray();

                            if (index >= arr.count) {
                                self.throwRuntimeError(
                                    "Index out of range",
                                    .{},
                                );
                                return .RuntimeError;
                            }

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError("failed to pop", .{});
                                return .RuntimeError;
                            };

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError("failed to pop", .{});
                                return .RuntimeError;
                            };

                            self.stack.push(arr.values.items[index]) catch {
                                self.throwRuntimeError(
                                    "Failed to push value to stack",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                        } else if (rawObj.asObj().isString()) {
                            const str = rawObj.asObj().asString();

                            if (index >= str.len) {
                                self.throwRuntimeError(
                                    "Index out of range",
                                    .{},
                                );
                                return .RuntimeError;
                            }

                            const charString = self.gc.copyString(
                                &[_]u32{str.chars[index]},
                                1,
                            ) catch {
                                self.throwRuntimeError(
                                    "failed to create a new string for string indexing",
                                    .{},
                                );
                                return .RuntimeError;
                            };

                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                            self.stack.push(PValue.makeObj(charString.parent())) catch {
                                self.throwRuntimeError(
                                    "Failed to push value to stack",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                        }
                    } else if (rawObj.asObj().isHmap()) {
                        const hmap = rawObj.asObj().asHmap();

                        if (hmap.getValue(rawIndex)) |val| {
                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                            _ = self.stack.pop() catch {
                                self.throwRuntimeError(
                                    "failed to pop",
                                    .{},
                                );
                                return .RuntimeError;
                            };

                            self.stack.push(val) catch {
                                self.throwRuntimeError(
                                    "Failed to push value to stack",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                        } else {
                            self.throwRuntimeError(
                                "Value couldn't be found for key : ",
                                .{},
                            );
                            _ = rawIndex.printVal(self.gc);
                            self.gc.pstdout.print(
                                "\n",
                                .{},
                            ) catch return .RuntimeError;
                            return .RuntimeError;
                        }
                    } else {
                        self.throwRuntimeError(
                            "Indexing only works on arrays and strings",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },
                .Op_Array => {
                    const itemCount = frame.readU16();
                    var i = itemCount;

                    const arr = self.gc.newObj(.Ot_Array, PObj.OArray) catch {
                        self.throwRuntimeError("Failed to create array", .{});
                        return .RuntimeError;
                    };

                    self.stack.push(PValue.makeObj(arr.parent())) catch {
                        self.throwRuntimeError("Failed to push", .{});
                        return .RuntimeError;
                    };

                    arr.init();

                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop array",
                            .{},
                        );
                        return .RuntimeError;
                    };
                    while (i > 0) : (i -= 1) {
                        if (!arr.addItem(self.gc, self.stack.pop() catch {
                            self.throwRuntimeError(
                                "Failed to pop array item",
                                .{},
                            );
                            return .RuntimeError;
                        })) {
                            self.throwRuntimeError(
                                "Failed to add item to array",
                                .{},
                            );
                            return .RuntimeError;
                        }
                    }

                    arr.reverseItems();
                    self.stack.push(PValue.makeObj(arr.parent())) catch {
                        self.throwRuntimeError(
                            "Failed to add array to stack",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },
                .Op_ClsUp => {
                    self.closeUpv(self.stack.top - 1);
                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed pop stack while closing upvalue",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_GetUp => {
                    const slot = frame.readRawByte();
                    self.stack.push(
                        frame.closure.upvalues[slot].location.*,
                    ) catch {
                        self.throwRuntimeError(
                            "Failed to push upvalue which getting it",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_SetUp => {
                    const slot = frame.readRawByte();

                    const v = self.peek(0);

                    frame.closure.upvalues[slot].location.* = v;
                },

                .Op_Closure => {
                    const func = frame.readConst().asObj().asFunc();
                    //_ = func.print(self.gc);
                    const cls = PObj.OClosure.new(self.gc, func) catch {
                        self.throwRuntimeError(
                            "Failed to create new closure for function",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    self.stack.push(cls.parent().asValue()) catch {
                        self.throwRuntimeError(
                            "Failed to push newly created closure",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    cls.globOwner = frame.globOwner;
                    cls.globals = &self.getModuleByHash(
                        frame.globOwner,
                    ).?.globals;
                    var i: usize = 0;
                    while (i < cls.upc) : (i += 1) {
                        const islocal = frame.readRawByte();
                        const index = frame.readRawByte();
                        if (islocal == 1) {
                            cls.upvalues[i] = self.captureUpval(
                                &frame.slots[index],
                            ) catch {
                                self.throwRuntimeError(
                                    "Failed to capture upvalues for closure",
                                    .{},
                                );
                                return .RuntimeError;
                            };
                        } else {
                            cls.upvalues[i] = frame.closure.upvalues[index];
                        }
                    }
                },

                .Op_Call => {
                    const argcount = frame.readRawByte();
                    if (!self.callVal(
                        self.peek(@intCast(argcount)),
                        argcount,
                    )) {
                        self.throwRuntimeError(
                            "Failed to call",
                            .{},
                        );
                        return .RuntimeError;
                    }
                    frame = &self.cmod.frames.stack[self.cmod.frameCount - 1];
                    //frame = &self.callframes.stack[self.callframes.count - 1];
                },

                .Op_JumpIfFalse => {
                    const offset = frame.readU16();
                    if (self.peek(0).isFalsy()) {
                        frame.ip += offset;
                    }
                },
                .Op_Jump => {
                    const offset = frame.readU16();
                    frame.ip += offset;
                },

                .Op_Loop => {
                    const offset = frame.readU16();
                    frame.ip -= offset;
                },
                .Op_EndMod => {
                    _ = self.stack.pop() catch return .RuntimeError;
                    self.cmod.frameCount -= 1;
                    self.cmod = self.cmod.origin.?;
                    frame = &self.cmod.frames.stack[self.cmod.frameCount - 1];
                    if (self.gc.compiler) |com| {
                        com.free(self.gc.getAlc());
                        self.gc.compiler = self.compiler;
                    }

                    continue;
                },
                .Op_Return => {
                    if (self.cmod.frameCount == 1 and self.cmod.isDefault) {
                        _ = self.stack.pop() catch {
                            self.throwRuntimeError(
                                "Failed to pop stack for return",
                                .{},
                            );
                            return .RuntimeError;
                        };
                        return .Ok;
                    }
                    const result = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop stack for return result",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    self.closeUpv(frame.slots);
                    self.cmod.frameCount -= 1;

                    self.stack.top = frame.slots;
                    self.stack.push(result) catch {
                        self.throwRuntimeError(
                            "Failed to push return result to stack",
                            .{},
                        );
                        return .RuntimeError;
                    };
                    //frame = &self.callframes.stack[self.callframes.count - 1];
                    frame = &self.cmod.frames.stack[self.cmod.frameCount - 1];
                },

                .Op_Show => {
                    const popVal = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "faild to pop stack for showing",
                            .{},
                        );
                        return .RuntimeError;
                    };
                    _ = popVal.printVal(self.gc);
                },

                .Op_Const => {
                    const con: PValue = frame.readConst();
                    self.stack.push(con) catch |err| {
                        self.throwRuntimeError(
                            "Failed to push constant to stack Because of {}",
                            .{err},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_Pop => {
                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop stack on op pop",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_Neg => {
                    var v = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop stack for neg",
                            .{},
                        );
                        return .RuntimeError;
                    };
                    if (v.isNumber()) {
                        self.stack.push(v.makeNeg()) catch {
                            self.throwRuntimeError(
                                "failed to push the neg value to stack",
                                .{},
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError(
                            "Negative only works on numbers",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_Add => {
                    if (!self.doBinaryOpAdd()) {
                        self.throwRuntimeError(
                            "Failed to do binary add",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_Sub => {
                    if (!self.doBinaryOpSub()) {
                        self.throwRuntimeError(
                            "Failed to do binary sub",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_Mul => {
                    if (!self.doBinaryOpMul()) {
                        self.throwRuntimeError(
                            "Failed to do binary mul",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_Pow => {
                    if (!self.doBinaryOpPow()) {
                        self.throwRuntimeError("Failed to do binary pow", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Div => {
                    if (!self.doBinaryOpDiv()) {
                        self.throwRuntimeError("Failed to do binary div", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Gt => {
                    if (!self.doBinaryOpComp(.C_Gt)) {
                        self.throwRuntimeError("Failed to do binary gt", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Lt => {
                    if (!self.doBinaryOpComp(.C_Lt)) {
                        self.throwRuntimeError("Failed to do binary lt", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Gte => {
                    if (!self.doBinaryOpComp(.C_Gte)) {
                        self.throwRuntimeError("Failed to do binary gte", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Lte => {
                    if (!self.doBinaryOpComp(.C_Lte)) {
                        self.throwRuntimeError("Failed to do binary lte", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Eq => {
                    if (!self.doValueComp(.C_Eq)) {
                        self.throwRuntimeError("Failed to do binary eq", .{});
                        return .RuntimeError;
                    }
                },

                .Op_Neq => {
                    if (!self.doValueComp(.C_Neq)) {
                        self.throwRuntimeError(
                            "Failed to do binary noteq",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_DefGlob => {
                    const name: *PObj.OString = frame.readStringConst();
                    frame.globals.put(
                        self.gc.hal(),
                        name,
                        self.peek(0),
                    ) catch {
                        self.throwRuntimeError(
                            "Failed to put value to globals table",
                            .{},
                        );
                        return .RuntimeError;
                    };

                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop value for def global",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_GetGlob => {
                    const name: *PObj.OString = frame.readStringConst();

                    if (frame.globals.get(name)) |value| {
                        self.stack.push(value) catch {
                            self.throwRuntimeError(
                                "failed to push value for get global",
                                .{},
                            );
                            return .RuntimeError;
                        };
                    } else if (self.gc.builtins.get(name)) |value| {
                        self.stack.push(value) catch {
                            self.throwRuntimeError(
                                "failed to push builtin function to stack",
                                .{},
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError("Undefined variable -> ", .{});
                        //_ = name.print(self.gc); //TO_DO

                        return .RuntimeError;
                    }
                },

                .Op_SetGlob => {
                    const name: *PObj.OString = frame.readStringConst();

                    if (frame.globals.get(name)) |_| {
                        //_ = self.gc.globals.fetchPut(self.gc.hal(), name, self.peek(0));
                        _ = self.cmod.globals.fetchPut(
                            self.gc.hal(),
                            name,
                            self.peek(0),
                        ) catch {
                            self.throwRuntimeError(
                                "failed to put value to globals in set global",
                                .{},
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError(
                            "Undefined variable",
                            .{},
                        );
                        return .RuntimeError;
                    }
                },

                .Op_GetLocal => {
                    const slot = frame.readRawByte();
                    self.stack.push(frame.slots[@intCast(slot)]) catch {
                        self.throwRuntimeError(
                            "failed to push value for get local",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_SetLocal => {
                    const slot = frame.readRawByte();
                    frame.slots[@intCast(slot)] = self.peek(0);
                },

                .Op_True => {
                    self.stack.push(PValue.makeBool(true)) catch {
                        self.throwRuntimeError(
                            "failed to push true value",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_False => {
                    self.stack.push(PValue.makeBool(false)) catch {
                        self.throwRuntimeError(
                            "failed to push false value",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                .Op_Nil => {
                    self.stack.push(PValue.makeNil()) catch {
                        self.throwRuntimeError("failed to push nil value", .{});
                        return .RuntimeError;
                    };
                },

                .Op_Not => {
                    const val = self.stack.pop() catch return .RuntimeError;
                    self.stack.push(PValue.makeBool(val.isFalsy())) catch {
                        self.throwRuntimeError(
                            "failed to push falsy value",
                            .{},
                        );
                        return .RuntimeError;
                    };
                },

                else => {
                    self.throwRuntimeError("unknown opcode found", .{});
                    self.gc.pstdout.print(
                        "OPCODE -> {any}",
                        .{op},
                    ) catch return .RuntimeError;
                    return IntrpResult.RuntimeError;
                },
            }
        }
    }
};
