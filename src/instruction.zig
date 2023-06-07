//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

const std = @import("std");
const PValue = @import("value.zig").PValue;

pub const OpCode = enum(u8) {
    Return,
    Const,
    Neg,
    Add,
    Sub,
    Mul,
    Div,
    Nil,
    True,
    False,
    Not,
    Eq,
    Gt,
    Gte,
    Lt,
    Lte,
    Show,
    Pop,
    DefGlob,
    SetGlob,
    GetGlob,
    GetLocal,
    SetLocal,
    JumpIfFalse,
    Jump,
    Loop,
    Call,
    Closure,
    GetUp,
    SetUp,
    ClsUp,
    Import,
    SetModProp,
    GetModProp,
    EndMod,
    Err,
    Array,
    Hmap,
    Index,
    SubAssign,

    const Self = @This();
    pub fn toString(self: *const Self) []const u8 {
        return switch (self.*) {
            .Return => "OP_RETURN",
            .Const => "OP_CONST",
            .Neg => "OP_NEG",
            .Add => "OP_ADD",
            .Sub => "OP_SUB",
            .Mul => "OP_MUL",
            .Div => "OP_DIV",
            .Nil => "OP_NIL",
            .True => "OP_TRUE",
            .False => "OP_FALSE",
            .Not => "OP_NOT",
            .Eq => "OP_EQ",
            .Gt => "OP_GT",
            .Gte => "OP_GTE",
            .Lt => "OP_LT",
            .Lte => "OP_LTE",
            .Show => "OP_SHOW",
            .Pop => "OP_POP",
            .DefGlob => "OP_DEF_GLOB",
            .SetGlob => "OP_SET_GLOB",
            .GetGlob => "OP_GET_GLOB",
            .GetLocal => "OP_GET_LOCAL",
            .SetLocal => "OP_SET_LOCAL",
            .JumpIfFalse => "OP_JUMP_IF_FALSE",
            .Jump => "OP_JUMP",
            .Loop => "OP_LOOP",
            .Call => "OP_CALL",
            .Closure => "OP_CLOSURE",
            .GetUp => "OP_GET_UP",
            .SetUp => "OP_SET_UP",
            .ClsUp => "OP_CLOSE_UP",
            .Import => "OP_IMPORT",
            .SetModProp => "OP_GET_MOD_PROP",
            .GetModProp => "OP_SET_MOD_PROP",
            .EndMod => "OP_END_MOD",
            .Err => "OP_ERR",
            .Array => "OP_ARRAY",
            .Hmap => "OP_HMAP",
            .Index => "OP_INDEX",
            .SubAssign => "OP_SUB_ASSIGN",
            //else => {"OP_UNKNOWN"; }
        };
    }
};

pub const InstPos = struct {
    virtual: bool,
    colpos: u32,
    line: u32,
    length: u32,

    pub fn dummy() InstPos {
        return InstPos{
            .virtual = true,
            .colpos = 0,
            .line = 0,
            .length = 0,
        };
    }
};

pub const Instruction = struct {
    code: std.ArrayList(u8),
    pos: std.ArrayList(InstPos),
    cons : std.ArrayList(PValue),
    pub fn init(al: std.mem.Allocator) Instruction {
        return Instruction{
            .code = std.ArrayList(u8).init(al),
            .pos = std.ArrayList(InstPos).init(al),
            .cons = std.ArrayList(PValue).init(al),
        };
    }

    pub fn free(self: *Instruction) void {
        self.code.deinit();
        self.pos.deinit();
        self.cons.deinit();
    }

    pub fn write_raw(self: *Instruction, bt: u8, pos: InstPos) !void {
        try self.code.append(bt);

        if (self.pos.items.len == 0 or
            self.pos.items[self.pos.items.len - 1].line != pos.line)
        {
            try self.pos.append(pos);
        }
    }

    pub fn write(self: *Instruction, bt: OpCode, pos: InstPos) !void {
        try self.code.append(@enumToInt(bt));

        if (self.pos.items.len == 0 or
            self.pos.items[self.pos.items.len - 1].line != pos.line)
        {
            try self.pos.append(pos);
        }
    }

    pub fn addConst(self : *Instruction , value : PValue) !void {
        try self.cons.append(value);// catch return false;
        //return true;
    }

    pub fn disasm(self: *Instruction, name: []const u8) void {
        std.debug.print("== {s} | [{any}] ==", .{ name, self.code.items.len });
        std.debug.print("\n", .{});

        var i: usize = 0;
        while (i < self.code.items.len) {
            i = self.disasmInstruction(i);
        }

        std.debug.print("\n", .{});
    }

    fn getOpCode(self: *Instruction, offset: usize) OpCode {
        return @intToEnum(OpCode, self.code.items[offset]);
    }

    fn disasmInstruction(self: *Instruction, offset: usize) usize {
        if (offset > 0 and self.pos.items[offset].line == self.pos.items[offset - 1].line) {
            std.debug.print(" | ", .{});
        } else {
            std.debug.print("{} ", .{offset});
        }

        const ins = self.getOpCode(offset);

        std.debug.print("{s}", .{ins.toString()});

        return offset + 1;
    }
};
