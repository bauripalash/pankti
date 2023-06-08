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
const vl = @import("value.zig");
const PValue = vl.PValue;

pub const IntrpResult = enum(u8) {
    Ok,
    CompileError,
    RuntimeError,
};

pub const Vm = struct {
    ins : ins.Instruction,
    al : std.mem.Allocator,
    ip : u8,
    stack : std.ArrayList(PValue),
    stackTop : usize,


    const Self = @This();

    pub fn newVm(al : std.mem.Allocator) Vm{
        return Vm{
            .al = al,
            .ins = undefined,
            .ip = 0,
            .stack = undefined,
            .stackTop = 0,
        };
    }

    pub fn bootVm(self : *Self) void {
        self.ins = ins.Instruction.init(self.al);
        self.stack = std.ArrayList(PValue).init(self.al);
    }

    pub fn freeVm(self : *Self) void {
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
    
    fn push(self : *Self , value : PValue) !void {
        try self.stack.append(value);
        
    }

    fn pop(self : *Self) PValue {
        return self.stack.pop();
    }

    pub fn debugStack(self : *Self) void{
        std.debug.print("==== STACK ====\n" , .{});
        for (self.stack.items, 0..) |value, i| {
            std.debug.print("[{} | " , .{i});
            value.printVal();
            std.debug.print("]\n" , .{});

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
                .Return => {
                    self.pop().printVal();
                    std.debug.print("\n" , .{});
                    return IntrpResult.Ok;
                },

                .Const => {
                   const con : PValue = self.readConst();
                   self.push(con) catch return .RuntimeError;

                },

                .Neg => {
                    var v = self.pop();
                    if (v.isNumber()) {
                        self.push(v.makeNeg()) catch return .RuntimeError;
                    } else {
                        return .RuntimeError;
                    }
                },

                .Add => {
                    if (!self.doBinaryOpAdd()){
                        return .RuntimeError;
                    }
                },

                .Sub => {
                    if (!self.doBinaryOpSub()) { return .RuntimeError; }
                },

                .Mul => { if (!self.doBinaryOpMul()) { return .RuntimeError; } 
                
                },

                .Div => {
                    if (!self.doBinaryOpDiv()) {
                        return .RuntimeError;
                    }
                },

                else => {
                    return IntrpResult.RuntimeError;
                }
            }
        }
    }

    pub fn interpret(self : *Self , inst : *ins.Instruction) IntrpResult{
        //@memcpy(self.ins.*, inst.);
        self.ip = 0;
        self.ins = inst.*;
        return self.run();
    }


};
