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
const PObj = @import("object.zig").PObj;
const ins = @import("instruction.zig");
const table = @import("table.zig");


const FRAME_MAX = 64;
const STACK_MAX = FRAME_MAX * std.math.maxInt(u8);

pub const StackError = error{
    StackOverflow,
    StackUnderflow,
    StackFailPush,
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
            _ = try self.pop();
        }
    }

    pub fn push(self: *Self, value: PValue) StackError!void {
        if (self.presentcount() >= STACK_MAX) {
            return StackError.StackOverflow;
        }
        self.top[0] = value;
        self.top += 1;
        self.count += 1;
    }

    pub fn pop(self: *Self) StackError!PValue {
        if (self.presentcount() == 0) {
            return StackError.StackUnderflow;
        }
        self.top -= 1;
        self.count -= 1;
        return self.top[0];
    }
};

pub const CallFrame = struct {
    closure: *PObj.OClosure,
    ip: [*]u8,
    slots: [*]PValue,
    globals : *table.PankTable(),
    globOwner : u32,

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

    pub inline fn readStringConst(self: *Self) *PObj.OString {
        return self.readConst().asObj().asString();
    }
};

pub const CallStack = struct {
    stack: [FRAME_MAX]CallFrame,
    count: u32 = 0,
};
