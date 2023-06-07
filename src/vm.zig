//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

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
        //self.stack.init
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
    
    fn push(self : *Self , value : PValue) bool {
        self.stack.append(value) catch return false;
        return true;
    }

    fn pop(self : *Self) PValue {
        return self.stack.pop();
    }

    fn run(self : *Self) IntrpResult{
        while (true) {
            const op = self.readByte();

            switch (op) {
                
                OpCode.Return => {
                    self.pop().printVal();
                    std.debug.print("\n" , .{});
                    return IntrpResult.Ok;
                },

                OpCode.Const => {
                    
                   const con : PValue = self.readConst();
                   _ = self.push(con);

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
