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
const Compiler = @import("compiler.zig").Compiler;
const vl = @import("value.zig");
const PValue = vl.PValue;
const Pobj = @import("object.zig").PObj;

pub const IntrpResult = enum(u8) {
    Ok,
    CompileError,
    RuntimeError,
};

pub const Vm = struct {
    ins : ins.Instruction,
    al : std.mem.Allocator,
    compiler : Compiler,
    ip : u8,
    stack : std.ArrayList(PValue),
    stackTop : usize,
    objects : ?*Pobj,


    const Self = @This();

    pub fn newVm(al : std.mem.Allocator) Vm{
        return Vm{
            .al = al,
            .ins = undefined,
            .ip = 0,
            .stack = undefined,
            .stackTop = 0,
            .compiler = undefined,
            .objects = null,
        };
    }

    pub fn bootVm(self : *Self) void {
        self.ins = ins.Instruction.init(self.al);
        self.stack = std.ArrayList(PValue).init(self.al);
        self.compiler.init(self);
    }

    fn freeObjects(self : *Self) void { 
        var obj = self.objects;
        while (obj != null) {
           const next = obj.?.next;
           obj.?.free(self);
            obj = next;

        }
    }

    pub fn freeVm(self : *Self) void {
        
       self.freeObjects(); 

        self.ins.free();
        self.stack.deinit();
    }

    fn readByte(self : *Self) ins.OpCode{
        self.ip += 1;
        return @intToEnum(ins.OpCode, self.ins.code.items[self.ip - 1]);

    }

    fn readRawByte(self : *Self) u8 {
        self.ip += 1;
        return self.ins.code.items[self.ip - 1];
    }

    fn readConst(self : *Self) PValue {
       return self.ins.cons.items[self.readRawByte()];
    }

    fn resetStack(self : *Self) void {
        self.stackTop = 0;
    }
    
    pub fn push(self : *Self , value : PValue) !void {
        try self.stack.append(value);
        
    }

    pub fn pop(self : *Self) PValue {
        return self.stack.pop();
    }


    fn peek(self : *Self , dist : usize) PValue{
        return self.stack.items[dist];
    }

    fn throwRuntimeError(self : *Self , msg : []const u8) void{
        
        std.debug.print("Runtime Error Occured in line {}", .{self.ins.pos.items[@intCast(usize, self.ip)-1].line});
        std.debug.print("\n{s}\n", .{msg});
    }

    pub fn debugStack(self : *Self) void{
        std.debug.print("==== STACK ====\n" , .{});
        if (self.stack.items.len > 0) {
            for (self.stack.items, 0..) |value, i| {
                const vs = value.toString(self.al) catch return;
                std.debug.print("[ |{:0>2}| {s:>4}" , .{self.stack.items.len - 1 - i , vs } );
                std.debug.print(" ]\n" , .{});
                self.al.free(vs);

            }
        }
        std.debug.print("===============\n\n" , .{});
    }

    fn doBinaryOpAdd(self : *Self) bool{
        // only works on numbers
        const b = self.pop();
        const a = self.pop();

        if (a.isNumber() and b.isNumber()) {
            self.push(PValue.makeNumber(a.asNumber() + b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpSub(self : *Self) bool{
        // only works on numbers
        const b = self.pop();
        const a = self.pop();

        if (a.isNumber() and b.isNumber()) {
            self.push(PValue.makeNumber(a.asNumber() - b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpMul(self : *Self) bool{
        // only works on numbers
        const b = self.pop();
        const a = self.pop();

        if (a.isNumber() and b.isNumber()) {
            self.push(PValue.makeNumber(a.asNumber() * b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn doBinaryOpDiv(self : *Self) bool{
        // only works on numbers
        const b = self.pop();
        const a = self.pop();

        if (a.isNumber() and b.isNumber()) {
            self.push(PValue.makeNumber(a.asNumber() / b.asNumber())) catch return false;
            return true;
        } else {
            return false;
        }
        
    }

    fn run(self : *Self) IntrpResult{
        while (true) {
            self.debugStack();
            const op = self.readByte();

            switch (op) {
                .Op_Return => {
                    //self.throwRuntimeError("Return occured");
                    self.pop().printVal();
                    std.debug.print("\n" , .{});
                    return IntrpResult.Ok;
                },

                .Op_Const => {
                   const con : PValue = self.readConst();
                   self.push(con) catch return .RuntimeError;

                },

                .Op_Neg => {
                    var v = self.pop();
                    if (v.isNumber()) {
                        self.push(v.makeNeg()) catch return .RuntimeError;
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

                .Op_Mul => { if (!self.doBinaryOpMul()) { return .RuntimeError; } 
                
                },

                .Op_Div => {
                    if (!self.doBinaryOpDiv()) {
                        return .RuntimeError;
                    }
                },

                .Op_True => {
                    self.push(PValue.makeBool(true)) catch {
                        return .RuntimeError;
                    };
                },

                .Op_False => {
                    self.push(PValue.makeBool(false)) catch {
                        return .RuntimeError;
                    };
                },

                .Op_Eq => {
                    const b = self.pop();
                    const a = self.pop();

                    self.push(PValue.makeBool(a.isEqual(b))) catch {
                        return .RuntimeError;  
                    };
                },

                .Op_Neq => {
                    const b = self.pop();
                    const a = self.pop();
                    
                    self.push(PValue.makeBool(!a.isEqual(b))) catch {
                        return .RuntimeError;  
                    };

                },

                .Op_Nil => {
                    self.push(PValue.makeNil()) catch {
                        return .RuntimeError;    
                    };
                },

                .Op_Not => {
                    self.push(PValue.makeBool(self.pop().isFalsy())) catch {
                        return .RuntimeError;
                    };
                },

                else => {
                    return IntrpResult.RuntimeError;
                }
            }
        }
    }

    pub fn interpretRaw(self : *Self , inst : *ins.Instruction) IntrpResult{
        //@memcpy(self.ins.*, inst.);
        self.ip = 0;
        self.ins = inst.*;
        return self.run();
    }

    pub fn interpret(self : *Self , source : []u32) IntrpResult{
        self.ip = 0;
        const result = self.compiler.compile(source, &self.ins) catch false;
        if (result) { 
            self.compiler.curIns().disasm("<script>");
            return self.interpretRaw(self.compiler.curIns());
        } else { 
            return .CompileError;
        }

    }


};
