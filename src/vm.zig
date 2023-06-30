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
const Gc = @import("gc.zig").Gc;
const vl = @import("value.zig");
const PValue = vl.PValue;
const Pobj = @import("object.zig").PObj;
const utils = @import("utils.zig");
const table = @import("table.zig");
const Allocator = std.mem.Allocator;
const flags = @import("flags.zig");
const builtins = @import("builtins.zig");

const FRAME_MAX = 64;
const STACK_MAX = FRAME_MAX * std.math.maxInt(u8);

pub const StackError = error{
    StackOverflow,
    StackUnderflow,
    StackFailPush,
};

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

pub const VStack = struct {
    stack: [STACK_MAX]PValue,
    head: [*]PValue,
    top: [*]PValue,
    count: usize,

    const Self = @This();

    pub fn presentcount(self: *Self) u64 {
        return (@as(
            u64,
            @intCast(@intFromPtr(self.top)),
        ) - @as(
            u64,
            @intCast(@intFromPtr(self.head)),
        )) / @sizeOf(*PValue);
    }

    pub fn clear(self: *Self) StackError!void {
        while (self.presentcount() != 0) {
            //std.debug.print("[Z] CLEARING STACK -> LEFT {}\n\n" , .{self.presentcount()});
            _ = try self.pop();
        }
    }

    pub fn push(self: *Self, value: PValue) StackError!void {
        if (self.presentcount() >= STACK_MAX) {
            //    std.debug.print("STACK -> {any}\n" , .{.{self.top[0] , self.count }} );
            return StackError.StackOverflow;
        }
        //const xvalue = self.presentcount();
        //std.debug.print("[X] push -> {} {} >< {}\n" , .{xvalue , self.count , STACK_MAX});

        //std.os.exit(0);

        self.top[0] = value;
        self.top += 1;
        self.count += 1;
    }

    pub fn pop(self: *Self) StackError!PValue {
        if (self.presentcount() == 0) {
            return StackError.StackUnderflow;
        }

        //std.debug.print("[Y] pop -> {} {} >< {}\n" , .{self.presentcount() , self.count , STACK_MAX});

        self.top -= 1;
        self.count -= 1;
        return self.top[0];
    }
};

pub const CallFrame = struct {
    closure: *Pobj.OClosure,
    ip: [*]u8,
    slots: [*]PValue,
    const Self = @This();

    pub inline fn readByte(self: *Self) ins.OpCode {
        const bt: ins.OpCode = @enumFromInt(self.ip[0]);

        self.ip += 1;

        return bt;
    }

    pub inline fn readU16(self: *Self) u16 {
        const b1 = self.readRawByte();
        const b2 = self.readRawByte();

        return (@as(u16, @intCast(b1)) << 8) | @as(u16, @intCast(b2));
    }

    pub inline fn readRawByte(self: *Self) u8 {
        const bt = self.ip[0];

        self.ip += 1;

        return bt;
    }

    pub inline fn readConst(self: *Self) PValue {
        return self.closure.function.ins.cons.items[self.readRawByte()];
        //return self.ins.cons.items[self.readRawByte()];
    }

    pub inline fn readStringConst(self: *Self) *Pobj.OString {
        return self.readConst().asObj().asString();
    }
};

pub const CallStack = struct {
    stack: [FRAME_MAX]CallFrame,
    count: u32 = 0,
};

pub const Vm = struct {
    callframes: CallStack,
    compiler: *Compiler,
    stack: VStack,
    gc: *Gc,

    const Self = @This();

    pub fn newVm(al: Allocator) !*Vm {
        const v = try al.create(Vm);
        return v;
    }

    pub fn bootVm(self: *Self, gc: *Gc) void {
        self.*.gc = gc;
        self.*.compiler = undefined;
        self.*.callframes.stack = undefined;
        self.*.callframes.count = 0;

        self.*.stack.stack[0] = PValue.makeNil();

        self.*.stack.count = 0;
        self.*.stack.top = self.stack.stack[0..];
        self.*.stack.head = self.stack.stack[0..];

        self.gc.stack = &self.stack;
        self.defineNative(
            &[_]u32{ 'c', 'l', 'o', 'c', 'k' },
            builtins.nClock,
        ) catch return;
        self.defineNative(
            &[_]u32{ 's', 'h', 'o', 'w' },
            builtins.nShow,
        ) catch return;

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

        const rfunc: ?*Pobj.OFunction = self.compiler.compile(
            source,
        ) catch return .CompileError;

        if (rfunc) |f| {
            self.stack.push(f.parent().asValue()) catch {
                std.debug.print("failed to push function to stack", .{});
                return .RuntimeError;
            };
            const cls = Pobj.OClosure.new(self.gc, f) catch {
                std.debug.print("failed to create a closure", .{});
                return .RuntimeError;
            };
            _ = self.stack.pop() catch return .RuntimeError;
            _ = self.stack.push(
                cls.parent().asValue(),
            ) catch return .RuntimeError;

            self.*.callframes.stack[0] = .{
                .closure = cls,
                .ip = f.ins.code.items.ptr,
                .slots = self.stack.stack[0..],
            };

            self.*.callframes.count = 1;
            self.gc.callstack = &self.callframes;

            return self.run();
        } else {
            return .RuntimeError;
        }
    }

    pub fn freeVm(self: *Self, al: Allocator) void {
        //std.debug.print("{d}\n", .{self.strings.keys().len});
        //_ = table.freeStringTable(self, self.strings);
        self.compiler.free(self.gc.getAlc());

        //self.stack.deinit(self.gc.getAlc());
        //self.ins.free();
        self.gc.free();
        //self.gc.getAlc().destroy(self);
        //self.gc.getAlc().destroy(self.gc);

        //self.stack.clear() catch {
        //    self.throwRuntimeError("failed to clear stack");
        //    return;
        //};
        self.resetStack();
        al.destroy(self);
    }

    fn resetStack(self: *Self) void {
        //self.stackTop = 0;
        self.stack.top = self.stack.stack[0..];
        self.stack.count = 0;
    }

    //pub fn push(self : *Self , value : PValue) StackError!void {
    //    self.stack.append(self.gc.getAlc() , value) catch {
    //        return StackError.StackFailPush;
    //    };
    //
    //}

    //pub fn pop(self : *Self) StackError!PValue {
    //    if (self.stack.items.len == 0) {
    //        self.throwRuntimeError("Stack Underflow Occured");
    //        return StackError.StackUnderflow;
    //    }
    //    return self.stack.pop();
    //}

    fn peek(self: *Self, dist: usize) PValue {
        return (self.stack.top - 1 - dist)[0];
    }

    fn throwRuntimeError(self: *Self, msg: []const u8) void {
        const frame = &self.callframes.stack[self.callframes.count - 1];
        const i = @intFromPtr(frame.ip) - @intFromPtr(
            frame.closure.function.ins.code.items.ptr,
        ) - 1;
        std.debug.print("Runtime Error Occured in line {}", .{
            frame.closure.function.ins.pos.items[i].line,
        });
        std.debug.print("\n{s}\n", .{msg});

        //std.debug.print("call frames -> {any}\n\n" , .{self.callframes});

        var j: i64 = @intCast(self.callframes.count - 1);
        //std.debug.print("CFC -> {d}\n" , .{self.callframes.count});
        while (j >= 0) {
            const f = &self.callframes.stack[@intCast(j)];
            const fun = f.closure.function;
            const instr = @intFromPtr(f.ip) - @intFromPtr(
                fun.ins.code.items.ptr,
            ) - 1;
            std.debug.print("[line {d}] in ", .{fun.ins.pos.items[instr].line});
            if (fun.name) |n| {
                utils.printu32(n.chars);
                std.debug.print("()\n", .{});
            } else {
                std.debug.print("<script>\n", .{});
            }

            j -= 1;
        }

        self.resetStack();
    }

    pub fn debugStack(self: *Self) void {
        std.debug.print("==== STACK ====\n", .{});
        if (self.stack.presentcount() > 0) {
            const presentcount = self.stack.presentcount();
            for (0..presentcount) |i| {
                const val = self.stack.stack[i];
                std.debug.print("[ |{:0>2}| {s:>4}", .{
                    presentcount - 1 - i,
                    " ",
                });
                val.printVal();
                std.debug.print(" ]\n", .{});
            }
        }
        std.debug.print("===============\n\n", .{});
    }

    fn defineNative(
        self: *Self,
        name: []const u32,
        func: Pobj.ONativeFunction.NativeFn,
    ) !void {
        const nstr = try self.gc.copyString(name, @intCast(name.len));

        try self.stack.push(nstr.parent().asValue());
        //self.debugStack();

        var nf = try self.gc.newObj(.Ot_NativeFunc, Pobj.ONativeFunction);

        nf.init(func);
        try self.stack.push(nf.parent().asValue());

        //nf.print();
        try self.gc.globals.put(
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
            self.gc.printTable(&self.gc.strings , "STRINGS");
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
                PValue.makeNumber(std.math.pow(f64, a.asNumber(), b.asNumber())),
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

    fn call(self: *Self, closure: *Pobj.OClosure, argc: u8) bool {
        if (argc != closure.function.arity) {
            self.throwRuntimeError("Function Expected Arg != Got");
            return false;
        }

        var frame = &self.callframes.stack[self.callframes.count];
        self.callframes.count += 1;
        frame.closure = closure;
        frame.ip = closure.function.ins.code.items.ptr;
        frame.slots = self.stack.top - argc - 1;
        return true;
    }

    fn callVal(self: *Self, calle: PValue, argc: u8) bool {
        if (calle.isObj()) {
            switch (calle.asObj().objtype) {
                .Ot_Closure => {
                    return self.call(calle.asObj().asClosure(), argc);
                },
                .Ot_NativeFunc => {
                    const f: *Pobj.ONativeFunction =
                        calle.asObj().asNativeFun();

                    const result = f.func(
                        argc,
                        (self.stack.top - argc)[0..argc],
                    );
                    self.stack.top -= argc + 1;
                    self.stack.push(result) catch return false;
                    return true;
                },
                else => {},
            }
        }

        //calle.printVal();
        self.throwRuntimeError("Can only call functions");
        return false;
    }

    fn captureUpval(self: *Self, local: *PValue) !*Pobj.OUpValue {
        var prevup: ?*Pobj.OUpValue = null;
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

        const newUp = try self.gc.newObj(.Ot_UpValue, Pobj.OUpValue);
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

    fn run(self: *Self) IntrpResult {
        var frame: *CallFrame =
            &self.callframes.stack[self.callframes.count - 1];

        while (true) {
            if (flags.DEBUG and flags.DEBUG_STACK) {
                self.debugStack();
            }

            if (flags.DEBUG and flags.DEBUG_GLOBS) {
                self.gc.printTable(&self.gc.globals , "GLOBS");
            }
            const op = frame.readByte();

            switch (op) {
                .Op_ClsUp => {
                    self.closeUpv(self.stack.top - 1);
                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed pop stack while closing upvalue",
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
                    const cls = Pobj.OClosure.new(self.gc, func) catch {
                        self.throwRuntimeError(
                            "Failed to create new closure for function",
                        );
                        return .RuntimeError;
                    };
                    self.stack.push(cls.parent().asValue()) catch {
                        self.throwRuntimeError(
                            "Failed to push newly created closure",
                        );
                        return .RuntimeError;
                    };

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
                        self.throwRuntimeError("Failed to call");
                        return .RuntimeError;
                    }
                    frame = &self.callframes.stack[self.callframes.count - 1];
                },

                .Op_JumpIfFalse => {
                    const offset = frame.readU16();
                    //std.debug.print("JIF -> {d}\n" , .{offset});
                    if (self.peek(0).isFalsy()) {
                        frame.ip += offset;
                    }
                },
                .Op_Jump => {
                    const offset = frame.readU16();
                    frame.ip += offset;

                    // std.debug.print("JIF -> {d}\n" , .{offset});
                },

                .Op_Loop => {
                    const offset = frame.readU16();
                    frame.ip -= offset;
                },
                .Op_Return => {
                    if (self.callframes.count == 1) {
                        _ = self.stack.pop() catch {
                            self.throwRuntimeError(
                                "Failed to pop stack for return",
                            );
                            return .RuntimeError;
                        };
                        return .Ok;
                    }
                    const result = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop stack for return result",
                        );
                        return .RuntimeError;
                    };

                    self.closeUpv(frame.slots);
                    self.callframes.count -= 1;

                    self.stack.top = frame.slots;
                    self.stack.push(result) catch {
                        self.throwRuntimeError(
                            "Failed to push return result to stack",
                        );
                        return .RuntimeError;
                    };
                    frame = &self.callframes.stack[self.callframes.count - 1];

                    //self.throwRuntimeError("Return occured");
                    //self.pop().printVal();
                    //std.debug.print("\n" , .{});
                    //const popVal = self.pop() catch return .RuntimeError;
                    //popVal.printVal();

                    //return IntrpResult.Ok;
                },

                .Op_Show => {
                    const popVal = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "faild to pop stack for showing",
                        );
                        return .RuntimeError;
                    };
                    std.debug.print("~~ ", .{});
                    popVal.printVal();
                    std.debug.print("\n", .{});
                },

                .Op_Const => {
                    const con: PValue = frame.readConst();
                    self.stack.push(con) catch |err| {
                        self.throwRuntimeError(
                            "Failed to push constant to stack",
                        );
                        std.debug.print("Because of {any}\n", .{err});
                        return .RuntimeError;
                    };
                },

                .Op_Pop => {
                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop stack on op pop",
                        );
                        return .RuntimeError;
                    };
                },

                .Op_Neg => {
                    var v = self.stack.pop() catch {
                        self.throwRuntimeError("Failed to pop stack for neg");
                        return .RuntimeError;
                    };
                    if (v.isNumber()) {
                        self.stack.push(v.makeNeg()) catch {
                            self.throwRuntimeError(
                                "failed to push the neg value to stack",
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError(
                            "Negative only works on numbers",
                        );
                        return .RuntimeError;
                    }
                },

                .Op_Add => {
                    if (!self.doBinaryOpAdd()) {
                        self.throwRuntimeError("Failed to do binary add");
                        return .RuntimeError;
                    }
                },

                .Op_Sub => {
                    if (!self.doBinaryOpSub()) {
                        self.throwRuntimeError("Failed to do binary sub");
                        return .RuntimeError;
                    }
                },

                .Op_Mul => {
                    if (!self.doBinaryOpMul()) {
                        self.throwRuntimeError("Failed to do binary mul");
                        return .RuntimeError;
                    }
                },

                .Op_Pow => {
                    if (!self.doBinaryOpPow()) {
                        self.throwRuntimeError("Failed to do binary pow");
                        return .RuntimeError;
                    }
                },

                .Op_Div => {
                    if (!self.doBinaryOpDiv()) {
                        self.throwRuntimeError("Failed to do binary div");
                        return .RuntimeError;
                    }
                },

                .Op_Gt => {
                    if (!self.doBinaryOpComp(.C_Gt)) {
                        self.throwRuntimeError("Failed to do binary gt");
                        return .RuntimeError;
                    }
                },

                .Op_Lt => {
                    if (!self.doBinaryOpComp(.C_Lt)) {
                        self.throwRuntimeError("Failed to do binary lt");
                        return .RuntimeError;
                    }
                },

                .Op_Gte => {
                    if (!self.doBinaryOpComp(.C_Gte)) {
                        self.throwRuntimeError("Failed to do binary gte");
                        return .RuntimeError;
                    }
                },

                .Op_Lte => {
                    if (!self.doBinaryOpComp(.C_Lte)) {
                        self.throwRuntimeError("Failed to do binary lte");
                        return .RuntimeError;
                    }
                },

                .Op_Eq => {
                    if (!self.doValueComp(.C_Eq)) {
                        self.throwRuntimeError("Failed to do binary eq");
                        return .RuntimeError;
                    }
                },

                .Op_Neq => {
                    if (!self.doValueComp(.C_Neq)) {
                        self.throwRuntimeError("Failed to do binary noteq");
                        return .RuntimeError;
                    }
                },

                .Op_DefGlob => {
                    const name: *Pobj.OString = frame.readStringConst();
                    self.gc.globals.put(
                        self.gc.hal(),
                        name,
                        self.peek(0),
                    ) catch {
                        self.throwRuntimeError(
                            "Failed to put value to globals table",
                        );
                        return .RuntimeError;
                    };

                    _ = self.stack.pop() catch {
                        self.throwRuntimeError(
                            "Failed to pop value for def global",
                        );
                        return .RuntimeError;
                    };
                },

                .Op_GetGlob => {
                    const name: *Pobj.OString = frame.readStringConst();

                    if (self.gc.globals.get(name)) |value| {
                        self.stack.push(value) catch {
                            self.throwRuntimeError(
                                "failed to push value for get global",
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError("Undefined variable");
                        return .RuntimeError;
                    }
                },

                .Op_SetGlob => {
                    const name: *Pobj.OString = frame.readStringConst();

                    if (self.gc.globals.get(name)) |_| {
                        //_ = self.gc.globals.fetchPut(self.gc.hal(), name, self.peek(0));
                        _ = self.gc.globals.fetchPut(
                            self.gc.hal(),
                            name,
                            self.peek(0),
                        ) catch {
                            self.throwRuntimeError(
                                "failed to put value to globals in set global",
                            );
                            return .RuntimeError;
                        };
                    } else {
                        self.throwRuntimeError("Undefined variable");
                        return .RuntimeError;
                    }
                },

                .Op_GetLocal => {
                    const slot = frame.readRawByte();
                    self.stack.push(frame.slots[@intCast(slot)]) catch {
                        self.throwRuntimeError(
                            "failed to push value for get local",
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
                        );
                        return .RuntimeError;
                    };
                },

                .Op_False => {
                    self.stack.push(PValue.makeBool(false)) catch {
                        self.throwRuntimeError(
                            "failed to push false value",
                        );
                        return .RuntimeError;
                    };
                },

                .Op_Nil => {
                    self.stack.push(PValue.makeNil()) catch {
                        self.throwRuntimeError("failed to push nil value");
                        return .RuntimeError;
                    };
                },

                .Op_Not => {
                    const val = self.stack.pop() catch return .RuntimeError;
                    self.stack.push(PValue.makeBool(val.isFalsy())) catch {
                        self.throwRuntimeError("failed to push falsy value");
                        return .RuntimeError;
                    };
                },

                else => {
                    self.throwRuntimeError("unknown opcode found");
                    std.debug.print("OPCODE -> {any}", .{op});
                    return IntrpResult.RuntimeError;
                },
            }
        }
    }
};
