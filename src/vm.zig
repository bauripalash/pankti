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

pub const StackError = error {
    StackOverflow,
    StackUnderflow,
    StackFailPush,
};

pub const IntrpResult = enum(u8) {
    Ok,
    CompileError,
    RuntimeError,

    pub fn toString(self : IntrpResult) []const u8 {
        switch (self) {
            .Ok => { return "Ok"; },
            .CompileError => { return "CompileError"; },
            .RuntimeError => { return "RuntimeError"; },
        }
    }
};


pub const VStack = struct {
    stack : [STACK_MAX]PValue,
    top : [*]PValue,
    count : u32,
    
    const Self = @This();

    pub fn push(self : *Self , value : PValue) StackError!void {
        
        if (self.count == STACK_MAX) {
            return StackError.StackOverflow;
        }

        self.top[0] = value;
        self.top += 1;
        self.count += 1;
    }

    pub fn pop(self : *Self) StackError!PValue {
        
        if (self.count == 0) {
            return StackError.StackUnderflow;
        }

        self.top -= 1;
        self.count -= 1;
        return self.top[0];
    }
};

pub const CallFrame = struct {
    closure : *Pobj.OClosure,
    ip : [*]u8,
    slots : [*]PValue,
    const Self = @This();
  
    pub inline fn readByte(self : *Self) ins.OpCode{
        const bt = @intToEnum(ins.OpCode, self.ip[0]);

        self.ip += 1;

        return bt;
    }

    pub inline fn readU16(self : *Self) u16 {
        const b1 = self.readRawByte();
        const b2 = self.readRawByte();

        return (@intCast(u16, b1) << 8) | @intCast(u16 , b2);
    }

    pub inline fn readRawByte(self : *Self) u8 {
        const bt =  self.ip[0];

        self.ip += 1;

        return bt;
    }

    pub inline fn readConst(self : *Self) PValue {
        return self.closure.function.ins.cons.items[self.readRawByte()];
       //return self.ins.cons.items[self.readRawByte()];
    }

    pub inline fn readStringConst(self : *Self) *Pobj.OString{
        return self.readConst().asObj().asString();

    }
};

pub const CallStack = struct {
    stack : [FRAME_MAX]CallFrame,
    count : u32 = 0,
};

pub const Vm = struct {
    callframes : CallStack,
    compiler : *Compiler,
    stack : VStack,
    gc : *Gc,


    const Self = @This();

    pub fn newVm(al : Allocator) !*Vm{
        const v = try al.create(Vm);
        return v;
        
    }

    pub fn bootVm(self : *Self , gc : *Gc) void {
        self.*.gc = gc;
        self.*.compiler = undefined;
        self.*.callframes = undefined;
    
        self.stack.top = self.stack.stack[0..];

        self.defineNative(&[_]u32{'c' , 'l' , 'o' , 'c' , 'k'} , builtins.nClock ) catch return;
        
        
        //self.*.ip = self.*.ins.code.items.ptr;
        

        //self.strings = table.StringTable(){};
        
    }

     pub fn interpretRaw(self : *Self , inst : *ins.Instruction) IntrpResult{
         _ = inst;
         _ = self;
        //@memcpy(self.ins.*, inst.);
        //self.ip = inst.code.items.ptr;
        //self.ins = inst.*;
        //return self.run();
    }

    pub fn interpret(self : *Self , source : []u32) IntrpResult{
        self.compiler = Compiler.new(source, self.gc , comp.FnType.Ft_SCRIPT) 
                        catch return .RuntimeError;

        const rfunc : ?*Pobj.OFunction = self.compiler.compile(source) catch return .CompileError;
        
        if (rfunc) |f| {
            self.stack.push(f.parent().asValue()) catch return .RuntimeError;
            const cls = Pobj.OClosure.new(self.gc, f) catch return .RuntimeError;
            _ = self.stack.pop() catch return .RuntimeError;
            _ = self.stack.push(cls.parent().asValue()) catch return .RuntimeError;
            
            self.*.callframes.stack[0] = .{
                .closure = cls,
                .ip = f.ins.code.items.ptr,
                .slots = self.stack.stack[0..]
            };

            self.*.callframes.count = 1;

            return self.run();
        } else {
            return .RuntimeError;
        }

       
    }

    pub fn freeVm(self : *Self , al : Allocator) void {
        //std.debug.print("{d}\n", .{self.strings.keys().len});
        //_ = table.freeStringTable(self, self.strings); 
        self.compiler.free(self.gc.getAlc());
        
        //self.stack.deinit(self.gc.getAlc());
        //self.ins.free();
        self.gc.free();
        al.destroy(self);
        
    }



    fn resetStack(self : *Self) void {
        //self.stackTop = 0;
        self.stack.top = self.stack.stack[0..];
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


    fn peek(self : *Self , dist : usize) PValue{
        return (self.stack.top - 1 - dist)[0];
    }

    fn throwRuntimeError(self : *Self , msg : []const u8) void{
        
        const frame = &self.callframes.stack[self.callframes.count - 1];
        const i = @ptrToInt(frame.ip) - @ptrToInt(frame.closure.function.ins.code.items.ptr) - 1;
        std.debug.print("Runtime Error Occured in line {}", .{frame.closure.function.ins.pos.items[i].line});
        std.debug.print("\n{s}\n", .{msg});

        //std.debug.print("call frames -> {any}\n\n" , .{self.callframes});
        
        var j : i64 = @intCast(i64 , self.callframes.count - 1);
        //std.debug.print("CFC -> {d}\n" , .{self.callframes.count});
        while (j >= 0) {
            const f = &self.callframes.stack[@intCast(usize , j)];
            const fun = f.closure.function;
            const instr = @ptrToInt(f.ip) - @ptrToInt(fun.ins.code.items.ptr) - 1;
            std.debug.print("[line {d}] in " , .{fun.ins.pos.items[instr].line});
            if (fun.name) |n| {
                utils.printu32(n.chars);
                std.debug.print("()\n" , .{});
            } else{
                std.debug.print("<script>\n" , .{});
            }

            j -= 1;
        }

        self.resetStack();
    }

    pub fn debugStack(self : *Self) void{
        std.debug.print("==== STACK ====\n" , .{});
        if (self.stack.items.len > 0) {
            for (self.stack.items, 0..) |value, i| {
                const vs = value.toString(self.gc.getAlc()) catch return;
                std.debug.print("[ |{:0>2}| {s:>4}" , .{self.stack.items.len - 1 - i , vs } );
                std.debug.print(" ]\n" , .{});
                self.gc.getAlc().free(vs);

            }
        }
        std.debug.print("===============\n\n" , .{});
    }

    fn defineNative(self : *Self , name : []const u32 , func : Pobj.ONativeFunction.NativeFn) !void {
        
        const nstr = try self.gc.copyString(name, @intCast(u32 , name.len));
        
        
        try self.stack.push(nstr.parent().asValue());
        var nf = try self.gc.newObj(.Ot_NativeFunc, Pobj.ONativeFunction);
        nf.init(func);

        try self.stack.push(nf.parent().asValue());

        try self.gc.globals.put(self.gc.getAlc() , self.stack.stack[0].asObj().asString() , self.stack.stack[1]);
        _ = try self.stack.pop();
        _ = try self.stack.pop();
        
    }

    fn doBinaryOpAdd(self : *Self) bool{
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeNumber(a.asNumber() + b.asNumber())) catch return false;
            return true;
        }else if (a.isString() and b.isString()) {
            const bs = b.asObj().asString();
            const as = b.asObj().asString();
            
            var temp_chars = self.gc.getAlc().alloc(u32, as.chars.len + bs.chars.len) catch return false;
            var i : usize = 0;

            while (i < as.chars.len) {
                temp_chars[i] = as.chars[i];
                i += 1;
            }

            while (i - as.chars.len < bs.chars.len) {
                temp_chars[i] = bs.chars[i - as.chars.len];
                i += 1;
            }
            

            const s = self.gc.copyString(temp_chars, @intCast(u32 , temp_chars.len)) catch {
                 self.gc.getAlc().free(temp_chars);
                return false;
            };
            self.stack.push(s.obj.asValue()) catch {
                self.gc.getAlc().free(temp_chars);
                return false;
            };
            self.gc.getAlc().free(temp_chars);
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpSub(self : *Self) bool{
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeNumber(a.asNumber() - b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpMul(self : *Self) bool{
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeNumber(a.asNumber() * b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpPow(self : *Self) bool{
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeNumber(std.math.pow(f64, a.asNumber() , b.asNumber()))) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpDiv(self : *Self) bool{
        // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeNumber(a.asNumber() / b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpComp(self : *Self) bool {
         // only works on numbers
        const b = self.stack.pop() catch return false;
        const a = self.stack.pop() catch return false;

        if (a.isNumber() and b.isNumber()) {
            self.stack.push(PValue.makeBool(a.asNumber() > b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
    }

    fn call(self : *Self , closure : *Pobj.OClosure , argc : u8) bool{
        
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

    fn callVal(self : *Self , calle : PValue , argc : u8) bool {

        if (calle.isObj()) {
            switch (calle.asObj().objtype) {
                .Ot_Closure => {
                    return self.call(calle.asObj().asClosure() , argc);
                },
                .Ot_NativeFunc => {
                    const f : *Pobj.ONativeFunction = calle.asObj().asNativeFun();
                    const result = f.func(argc , (self.stack.top - argc)[0..argc]);
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

    fn captureUpval(self : *Self , local : *PValue) !*Pobj.OUpValue{

        var prevup : ?*Pobj.OUpValue = null;
        var upval = self.gc.openUps;

        while (upval != null and @ptrToInt(upval.?.location) > @ptrToInt(local)) {
            prevup = upval;
            upval = upval.?.next;
        }

        if (upval != null and @ptrToInt(upval.?.location) == @ptrToInt(local)) {
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

    fn closeUpv(self : *Self , last : [*]PValue) void{
        while (self.gc.openUps) |oup| {
            if (!(@ptrToInt(oup) >= @ptrToInt(last))) {
                break;
            }

            var upv = oup;
            upv.closed = oup.location.*;
            upv.location = &upv.closed;
            self.gc.openUps = upv.next;

        }
    }


    fn run(self : *Self) IntrpResult{
        
        var frame : *CallFrame = 
            &self.callframes.stack[self.callframes.count - 1];

        while (true) {
            if (flags.DEBUG and flags.DEBUG_STACK) {
                self.debugStack();
            }
            const op = frame.readByte();

            switch (op) {

                .Op_ClsUp => {
                    self.closeUpv(self.stack.top - 1);
                    _ = self.stack.pop() catch return .RuntimeError;
                },

                .Op_GetUp => {
                    const slot = frame.readRawByte();
                    self.stack.push(frame.closure.upvalues[slot].location.*) catch 
                        return .RuntimeError;
                },

                .Op_SetUp => {

                    const slot = frame.readRawByte();

                    const v = self.peek(0);

                    frame.closure.upvalues[slot].location.* = v;
                },

                .Op_Closure => {
                    const func = frame.readConst().asObj().asFunc();
                    const cls = Pobj.OClosure.new(self.gc, func) catch return .RuntimeError;
                    self.stack.push(cls.parent().asValue()) catch return .RuntimeError;

                    var i : usize = 0;
                    while (i < cls.upc) : (i += 1) {
                        const islocal = frame.readRawByte();
                        const index = frame.readRawByte();
                        if (islocal == 1) {
                            cls.upvalues[i] = self.captureUpval(&frame.slots[index]) catch return .RuntimeError;
                        } else {
                            cls.upvalues[i] = frame.closure.upvalues[index];
                        }
                    }
                },

                .Op_Call => {
                    const argcount = frame.readRawByte();
                    if (!self.callVal(self.peek(@intCast(usize , argcount)), argcount)) {
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
                        _ = self.stack.pop() catch return .RuntimeError;
                        return .Ok;
                    }
                    const result = self.stack.pop() catch return .RuntimeError;

                    self.closeUpv(frame.slots);
                    self.callframes.count -= 1;
                    

                    self.stack.top = frame.slots;
                    self.stack.push(result) catch return .RuntimeError;
                    frame = &self.callframes.stack[self.callframes.count - 1];

                    //self.throwRuntimeError("Return occured");
                    //self.pop().printVal();
                    //std.debug.print("\n" , .{});
                    //const popVal = self.pop() catch return .RuntimeError;
                    //popVal.printVal();

                    //return IntrpResult.Ok;
                },

                .Op_Show => {
                    const popVal = self.stack.pop() catch return .RuntimeError;
                    std.debug.print("~~ " , .{});
                    popVal.printVal();
                    std.debug.print("\n", .{});
                },

                .Op_Const => {
                   const con : PValue = frame.readConst();
                   self.stack.push(con) catch return .RuntimeError;

                },

                .Op_Pop => {
                    _ = self.stack.pop() catch return .RuntimeError;
                },

                .Op_Neg => {
                    var v = self.stack.pop() catch return .RuntimeError;
                    if (v.isNumber()) {
                        self.stack.push(v.makeNeg()) catch return .RuntimeError;
                    } else {
                        return .RuntimeError;
                    }
                },

                .Op_Add => {
                    if (!self.doBinaryOpAdd()){
                        return .RuntimeError;
                    }
                },

                .Op_Sub => {
                    if (!self.doBinaryOpSub()) { return .RuntimeError; }
                },

                .Op_Mul => { 
                    if (!self.doBinaryOpMul()) { return .RuntimeError; } 
                
                },

                .Op_Pow => {
                    if (!self.doBinaryOpPow()) { return .RuntimeError; }
                },

                .Op_Div => {
                    if (!self.doBinaryOpDiv()) {
                        return .RuntimeError;
                    }
                },

                .Op_Gt => {

                    if (!self.doBinaryOpComp()){
                        return .RuntimeError;
                    }
                
                },

                .Op_DefGlob => {
                    const name : *Pobj.OString = frame.readStringConst();
                    self.gc.globals.put(self.gc.getAlc(), name, self.peek(0))
                        catch return .RuntimeError;

                     _ = self.stack.pop() catch return .RuntimeError;
                },

                .Op_GetGlob => {
                    const name : *Pobj.OString = frame.readStringConst();

                    if (self.gc.globals.get(name)) |value| {
                        self.stack.push(value) catch return .RuntimeError;
                    }else{
                        self.throwRuntimeError("Undefined variable");
                        return .RuntimeError;
                    }
                },

                .Op_SetGlob => {
                     const name : *Pobj.OString = frame.readStringConst();

                    if (self.gc.globals.get(name)) |_| {
                        self.gc.globals.put(self.gc.getAlc(),
                                            name, 
                                            self.peek(0)) catch 
                                                return .RuntimeError;
                    }else{
                        self.throwRuntimeError("Undefined variable");
                        return .RuntimeError;
                    }
                },

                .Op_GetLocal => {
                    const slot = frame.readRawByte();
                    self.stack.push(frame.slots[@intCast(usize , slot)]) catch 
                        return .RuntimeError;
                },

                .Op_SetLocal => {

                    const slot = frame.readRawByte();
                    frame.slots[@intCast(usize , slot)] = self.peek(0);
                },

                .Op_True => {
                    self.stack.push(PValue.makeBool(true)) catch {
                        return .RuntimeError;
                    };
                },

                .Op_False => {
                    self.stack.push(PValue.makeBool(false)) catch {
                        return .RuntimeError;
                    };
                },

                .Op_Eq => {
                    const b = self.stack.pop() catch return .RuntimeError;
                    const a = self.stack.pop() catch return .RuntimeError;

                    self.stack.push(PValue.makeBool(a.isEqual(b))) catch {
                        return .RuntimeError;  
                    };
                },

                .Op_Neq => {
                    const b = self.stack.pop() catch return .RuntimeError;
                    const a = self.stack.pop() catch return .RuntimeError;
                    
                    self.stack.push(PValue.makeBool(!a.isEqual(b))) catch {
                        return .RuntimeError;  
                    };

                },

                .Op_Nil => {
                    self.stack.push(PValue.makeNil()) catch {
                        return .RuntimeError;    
                    };
                },

                .Op_Not => {
                    const val = self.stack.pop() catch return .RuntimeError;
                    self.stack.push(PValue.makeBool(val.isFalsy())) catch {
                        return .RuntimeError;
                    };
                },

                else => {
                    return IntrpResult.RuntimeError;
                }
            }
        }
    }

   


};
