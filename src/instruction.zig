//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const PValue = @import("value.zig").PValue;
const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const PObj = @import("object.zig").PObj;

pub const OpCode = enum(u8) {
    Op_Return,
    Op_Const,
    Op_Neg,
    Op_Add,
    Op_Sub,
    Op_Mul,
    Op_Div,
    Op_Nil,
    Op_True,
    Op_False,
    Op_Not,
    Op_Eq,
    Op_Gt,
    Op_Gte,
    Op_Lt,
    Op_Lte,
    Op_Pop,
    Op_DefGlob,
    Op_SetGlob,
    Op_GetGlob,
    Op_GetLocal,
    Op_SetLocal,
    Op_JumpIfFalse,
    Op_Jump,
    Op_Loop,
    Op_Call,
    Op_Closure,
    Op_GetUp,
    Op_SetUp,
    Op_ClsUp,
    Op_Import,
    Op_SetModProp,
    Op_GetModProp,
    Op_EndMod,
    Op_Err,
    Op_Array,
    Op_Hmap,
    Op_Index,
    Op_SubAssign,

    Op_Show,
    Op_Neq,
    Op_Pow,

    const Self = @This();
    pub fn toString(self: *const Self) []const u8 {
        return switch (self.*) {
            .Op_Return => "OP_RETURN",
            .Op_Const => "OP_CONST",
            .Op_Neg => "OP_NEG",
            .Op_Add => "OP_ADD",
            .Op_Sub => "OP_SUB",
            .Op_Mul => "OP_MUL",
            .Op_Pow => "OP_POW",
            .Op_Div => "OP_DIV",
            .Op_Nil => "OP_NIL",
            .Op_True => "OP_TRUE",
            .Op_False => "OP_FALSE",
            .Op_Not => "OP_NOT",
            .Op_Eq => "OP_EQ",
            .Op_Neq => "OP_NEQ",
            .Op_Gt => "OP_GT",
            .Op_Gte => "OP_GTE",
            .Op_Lt => "OP_LT",
            .Op_Lte => "OP_LTE",
            .Op_Show => "OP_SHOW",
            .Op_Pop => "OP_POP",
            .Op_DefGlob => "OP_DEF_GLOB",
            .Op_SetGlob => "OP_SET_GLOB",
            .Op_GetGlob => "OP_GET_GLOB",
            .Op_GetLocal => "OP_GET_LOCAL",
            .Op_SetLocal => "OP_SET_LOCAL",
            .Op_JumpIfFalse => "OP_JUMP_IF_FALSE",
            .Op_Jump => "OP_JUMP",
            .Op_Loop => "OP_LOOP",
            .Op_Call => "OP_CALL",
            .Op_Closure => "OP_CLOSURE",
            .Op_GetUp => "OP_GET_UP",
            .Op_SetUp => "OP_SET_UP",
            .Op_ClsUp => "OP_CLOSE_UP",
            .Op_Import => "OP_IMPORT",
            .Op_SetModProp => "OP_GET_MOD_PROP",
            .Op_GetModProp => "OP_SET_MOD_PROP",
            .Op_EndMod => "OP_END_MOD",
            .Op_Err => "OP_ERR",
            .Op_Array => "OP_ARRAY",
            .Op_Hmap => "OP_HMAP",
            .Op_Index => "OP_INDEX",
            .Op_SubAssign => "OP_SUB_ASSIGN",
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

    pub fn line(l: u32) InstPos {
        return InstPos{
            .virtual = true,
            .colpos = 0,
            .line = l,
            .length = 0,
        };
    }
};

pub const Instruction = struct {
    code: std.ArrayListUnmanaged(u8),
    pos: std.ArrayListUnmanaged(InstPos),
    cons: std.ArrayListUnmanaged(PValue),
    gc: *Gc,

    pub fn init(gc: *Gc) Instruction {
        return Instruction{
            .code = std.ArrayListUnmanaged(u8){},
            .pos = std.ArrayListUnmanaged(InstPos){},
            .cons = std.ArrayListUnmanaged(PValue){},
            .gc = gc,
        };
    }

    pub fn makeChangesForModule(self : *Instruction) bool {
        const len = self.code.items.len;
        if (len < 2) return false;
        self.code.items[len - 1] = @intFromEnum(OpCode.Op_EndMod);
        return true;
    }

    pub fn free(self: *Instruction) void {
        self.code.deinit(self.gc.hal());
        self.pos.deinit(self.gc.hal());
        self.cons.deinit(self.gc.hal());
    }

    pub fn write_raw(self: *Instruction, bt: u8, pos: InstPos) !void {
        try self.code.append(self.gc.hal(), bt);
        try self.pos.append(self.gc.hal(), pos);
    }

    pub fn write(self: *Instruction, bt: OpCode, pos: InstPos) !void {
        try self.code.append(self.gc.hal(), @intFromEnum(bt));
        try self.pos.append(self.gc.hal(), pos);
    }

    pub fn addConst(self: *Instruction, value: PValue) !u8 {
        try self.cons.append(self.gc.hal(), value);
        return @intCast(self.cons.items.len - 1);
        // catch return false;
        //return true;
    }

    /// Return OpCode at offset
    fn getOpCode(self: *Instruction, offset: usize) OpCode {
        return @enumFromInt(self.code.items[offset]);
    }

    /// Return OpCode at offset
    fn getRawOpCode(self: *Instruction, offset: usize) u8 {
        return self.code.items[offset];
    }

    pub fn disasm(self: *Instruction, name: []const u8) void {
        self.gc.pstdout.print("== {s} | [{any}] ==", .{ name, self.code.items.len }) catch return;
        self.gc.pstdout.print("\n", .{}) catch return;

        var i: usize = 0;
        while (i < self.code.items.len) {
            i = self.disasmInstruction(i);
        }

        self.gc.pstdout.print("\n", .{}) catch return;
    }

    fn simpleInstruction(
        self : *Instruction,
        name: []const u8,
        offset: usize,
    ) usize {
        self.gc.pstdout.print("{s}\n", .{name}) catch return 0;
        return offset + 1;
    }

    fn constInstruction(
        self: *Instruction,
        name: []const u8,
        offset: usize,
    ) usize {
        const constIndex = self.getRawOpCode(offset + 1);
        self.gc.pstdout.print("{s} {d} '", .{ name, constIndex }) catch return 0;
        _ = self.cons.items[constIndex].printVal(self.gc);
        self.gc.pstdout.print("'\n", .{}) catch return 0 ;

        return offset + 2;
    }

    fn jumpInstruction(
        self: *Instruction,
        name: []const u8,
        sign: i32,
        offset: usize,
    ) usize {
        var jump: u16 = utils.u8tou16(&[_]u8{
            self.code.items[offset + 1],
            self.code.items[offset + 2],
        });
        self.gc.pstdout.print("{s} {d} -> {d}\n", .{
            name,
            offset,
            @as(i64, @intCast(offset)) + 3 + sign * jump,
        }) catch return 0 ;
        return offset + 3;
    }

    fn byteInstruction(
        self: *Instruction,
        name: []const u8,
        offset: usize,
    ) usize {
        const slot = self.code.items[offset + 1];
        self.gc.pstdout.print("{s} {:>4}\n", .{ name, slot }) catch return 0;
        return offset + 2;
    }

    fn disasmInstruction(self: *Instruction, offset: usize) usize {
        self.gc.pstdout.print("{:0>4} ", .{offset}) catch return 0;
        if (offset > 0 and self.pos.items[offset].line == self.pos.items[offset - 1].line) {
            self.gc.pstdout.print("   | ", .{}) catch return 0;
        } else {
            self.gc.pstdout.print("{:>4} ", .{self.pos.items[offset].line}) catch return 0;
        }

        const ins = self.getOpCode(offset);

        switch (ins) {
            .Op_Return,
            .Op_Neg,
            .Op_Add,
            .Op_Sub,
            .Op_Mul,
            .Op_Pow,
            .Op_Div,
            .Op_Nil,
            .Op_True,
            .Op_False,
            .Op_Not,
            .Op_Eq,
            .Op_Neq,
            .Op_Lt,
            .Op_Gt,
            .Op_Pop,
            .Op_ClsUp,
            .Op_Err,
            .Op_Index,
            .Op_Show,
            .Op_SubAssign,
            => {
                return self.simpleInstruction(ins.toString(), offset);
            },

            .Op_SetUp, .Op_GetUp, .Op_GetLocal, .Op_SetLocal => {
                return self.byteInstruction(ins.toString(), offset);
            },

            .Op_JumpIfFalse, .Op_Jump => {
                return self.jumpInstruction(ins.toString(), 1, offset);
            },

            .Op_Loop => {
                return self.jumpInstruction(ins.toString(), -1, offset);
            },

            .Op_Call => {
                return self.byteInstruction(ins.toString(), offset);
            },

            .Op_Const,
            .Op_Import,
            .Op_DefGlob,
            .Op_GetGlob,
            .Op_SetGlob,
            => {
                return self.constInstruction(ins.toString(), offset);
            },

            .Op_Array , .Op_Hmap => {
                var con1 = self.code.items[offset + 1];
                var con2 = self.code.items[offset + 2];
                self.gc.pstdout.print("{s} {d}\n" , .{ins.toString() , utils.u8tou16(&[_]u8{con1 , con2})}) catch return 0;
                return offset + 3;
            },

            .Op_Closure => {
                var off = offset + 1;
                const con = self.code.items[off];
                off += 1;
                self.gc.pstdout.print("{s} {d} ", .{ ins.toString(), con }) catch return 0;
                _ = self.cons.items[con].printVal(self.gc);
                self.gc.pstdout.print("\n", .{}) catch return 0;

                const f: *PObj.OFunction =
                    self.cons.items[con].asObj().asFunc();
                var i: usize = 0;
                while (i < f.upvCount) : (i += 1) {
                    const isLocal = self.code.items[off];
                    off += 1;
                    const index = self.code.items[off];
                    off += 1;
                    self.gc.pstdout.print("{:0>4}   |   ->", .{off - 2}) catch return 0;
                    if (isLocal == 1) {
                        self.gc.pstdout.print("local", .{}) catch return 0;
                    } else {
                        self.gc.pstdout.print("upvalue", .{}) catch return 0;
                    }

                    self.gc.pstdout.print(" {d}\n", .{index}) catch return 0;
                }

                return off;
            },
            else => {
                return offset + 1;
            },
        }
    }
};
