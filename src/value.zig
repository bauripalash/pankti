//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const BN_INFINITY = "অসীম";
const BN_NAN = "অসংজ্ঞাত";
const BN_TRUE = "সত্যি";
const BN_FALSE = "মিথ্যা";
const BN_NIL = "নিল";
const BN_UNKNOWN = "অজানা মান";

const std = @import("std");
const PObj = @import("object.zig").PObj;
const Gc = @import("gc.zig").Gc;
const writer = @import("writer.zig");
const utils = @import("utils.zig");
const BnName = @import("bengali/names.zig");
const math = std.math;
const valueerrors = @import("value_errors.zig");
const CopyError = valueerrors.CopyError;

pub const ParentLink = struct {
    prev: std.ArrayListUnmanaged(PValue),

    pub fn exists(self: *ParentLink, v: *PObj) bool {
        for (self.prev.items) |item| {
            if (item.isObj()) {
                if (@intFromPtr(v) == @intFromPtr(item.asObj()) and item.asObj().getType() == v.getType()) {
                    return true;
                }
            }
        }

        return false;
    }

    pub fn free(self: *ParentLink, gc: *Gc) bool {
        self.prev.clearAndFree(gc.hal());
        self.prev.deinit(gc.hal());
        gc.hal().destroy(self);
        return true;
    }
};

pub const PValueType = enum(u8) {
    Pt_Num,
    Pt_Bool,
    Pt_Nil,
    Pt_Obj,
    Pt_Unknown,

    pub fn toSimpleString(self: PValueType) []const u8 {
        switch (self) {
            .Pt_Num => return BnName.simpleNameNumber,
            .Pt_Bool => return BnName.simpleNameBool,
            .Pt_Nil => return BnName.simpleNameNil,
            .Pt_Obj => return BnName.simpleNameObject,
            .Pt_Unknown => return BnName.simpleNameUnknown,
        }
    }

    pub fn toString(self: PValueType) []const u8 {
        switch (self) {
            .Pt_Num => return "VAL_NUM",
            .Pt_Bool => return "VAL_BOOL",
            .Pt_Nil => return "VAL_NIL",
            .Pt_Obj => return "VAL_OBJ",
            .Pt_Unknown => return "VAL_UNKNOWN",
        }
    }
};

pub const PValue = packed struct {
    data: u64,
    const QNAN: u64 = 0x7ffc000000000000;
    const SIGN_BIT: u64 = 0x8000000000000000;

    const TAG_NIL = 1;
    const TAG_FALSE = 2;
    const TAG_TRUE = 3;

    const NIL_VAL = PValue{ .data = QNAN | TAG_NIL };
    const FALSE_VAL = PValue{ .data = QNAN | TAG_FALSE };
    const TRUE_VAL = PValue{ .data = QNAN | TAG_TRUE };

    const Self = @This();

    pub fn getLen(self: Self) ?usize {
        if (self.isObj()) return self.asObj().getLen();
        return null;
    }
    pub fn hash(self: Self) u32 {
        const data = self.data;
        var result: u32 = 0;

        if (utils.IS_WASM) {
            var hasher = std.hash.Fnv1a_32.init();
            std.hash.autoHash(&hasher, data);
            result = hasher.final();
        } else {
            var hasher = std.hash.XxHash32.init(@intCast(std.time.timestamp()));
            std.hash.autoHash(&hasher, data);
            result = hasher.final();
        }

        return result;
    }

    pub fn createCopy(
        self: Self,
        gc: *Gc,
        links: ?*ParentLink,
    ) CopyError!PValue {
        if (self.isObj()) {
            const obj = self.asObj().createCopy(gc, links) catch |e| {
                return e;
            };

            return PValue.makeObj(obj);
        } else {
            return self;
        }
    }

    /// is value a bool
    pub fn isBool(self: Self) bool {
        return (self.data | 1) == TRUE_VAL.data;
    }

    /// is value a nil
    pub fn isNil(self: Self) bool {
        return self.data == NIL_VAL.data;
    }

    /// is value a nil
    pub fn isNumber(self: Self) bool {
        return (self.data & QNAN) != QNAN;
    }

    /// is value a object
    pub fn isObj(self: Self) bool {
        return (self.data & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT);
    }

    pub fn isMod(self: Self) bool {
        if (!self.isObj()) return false;
        if (!self.asObj().isMod()) return false;
        return true;
    }

    pub fn isError(self: Self) bool {
        return self.isObj() and self.asObj().isOError();
    }

    pub fn isString(self: Self) bool {
        if (self.isObj()) {
            return self.asObj().isString();
        }

        return false;
    }

    pub fn makeComptimeError(
        gc: *Gc,
        comptime msg: []const u8,
        args: anytype,
    ) ?PValue {
        const rawO = gc.newObj(.Ot_Error, PObj.OError) catch return null;
        rawO.parent().isMarked = true;
        if (!rawO.initU8Args(gc, msg, args)) return null;

        const val = PValue.makeObj(rawO.parent());
        rawO.parent().isMarked = false;
        return val;
    }

    pub fn makeError(gc: *Gc, msg: []const u8) ?PValue {
        const rawO = gc.newObj(.Ot_Error, PObj.OError) catch return null;
        rawO.parent().isMarked = true;
        if (!rawO.initU8(gc, msg)) return null;
        const val = PValue.makeObj(rawO.parent());
        rawO.parent().isMarked = false;
        return val;
    }

    /// get a number value as `f64`
    pub fn asNumber(self: Self) f64 {
        if (self.isNumber()) {
            return @bitCast(self.data);
        } else {
            return 0;
        }
    }

    /// get a bool value as `bool`
    pub fn asBool(self: Self) bool {
        if (self.isBool()) {
            return self.data == TRUE_VAL.data;
        } else {
            return false;
        }
    }

    pub fn asObj(self: Self) *PObj {
        const v: usize = @intCast(self.data & ~(SIGN_BIT | QNAN));

        return @ptrFromInt(v);
    }

    pub fn asDataObj(self: Self) usize {
        return @as(usize, @intCast(self.data & ~(SIGN_BIT | QNAN)));
    }

    /// Create a new number value
    pub fn makeNumber(n: f64) PValue {
        return PValue{ .data = @bitCast(n) };
    }

    /// Create a new bool value
    pub fn makeBool(b: bool) PValue {
        if (b) {
            return TRUE_VAL;
        } else {
            return FALSE_VAL;
        }
    }

    /// Create a new nil value
    pub fn makeNil() PValue {
        return NIL_VAL;
    }

    pub fn makeObj(o: *PObj) PValue {
        return PValue{
            .data = SIGN_BIT | QNAN | @intFromPtr(o),
        };
    }

    /// Return a new value with with negative value of itself;
    /// If `self` is not a number return itself
    pub fn makeNeg(self: Self) PValue {
        if (self.isNumber()) {
            return PValue.makeNumber(-self.asNumber());
        } else {
            return self;
        }
    }

    pub fn getType(self: Self) PValueType {
        if (self.isNumber()) {
            return .Pt_Num;
        } else if (self.isBool()) {
            return .Pt_Bool;
        } else if (self.isNil()) {
            return .Pt_Nil;
        } else if (self.isObj()) {
            return .Pt_Obj;
        }

        return .Pt_Unknown;
    }

    pub fn getTypeAsString(self: Self) []const u8 {
        switch (self.getType()) {
            .Pt_Num,
            .Pt_Bool,
            .Pt_Nil,
            .Pt_Unknown,
            => return self.getType().toString(),
            .Pt_Obj => {
                return self.asObj().getType().toString();
            },
        }
    }

    pub fn getTypeAsSimpleStr(self: Self) []const u8 {
        switch (self.getType()) {
            .Pt_Num,
            .Pt_Bool,
            .Pt_Nil,
            .Pt_Unknown,
            => return self.getType().toSimpleString(),
            .Pt_Obj => {
                return self.asObj().getType().toSimpleString();
            },
        }
    }

    pub fn isEqual(self: Self, other: PValue) bool {
        if (self.getType() != other.getType()) {
            return false;
        }

        if (self.isBool()) {
            return self.asBool() == other.asBool();
        } else if (self.isNumber()) {
            return self.asNumber() == other.asNumber();
        } else if (self.isNil()) {
            return true;
        } else if (self.isObj()) {
            return self.asObj().isEqual(other.asObj());
        }

        return false;
    }

    /// Chec if value is falsy
    pub fn isFalsy(self: Self) bool {
        if (self.isBool()) {
            return !self.asBool();
        } else if (self.isNil()) {
            return true;
        } else {
            return false;
        }
    }

    /// Print value of PValue to console
    pub fn printVal(self: Self, gc: *Gc, link: ?*ParentLink) bool {
        if (self.isNil()) {
            gc.pstdout.print(BN_NIL, .{}) catch return false;
        } else if (self.isBool()) {
            const b: bool = self.asBool();
            if (b) {
                gc.pstdout.print(BN_TRUE, .{}) catch return false;
            } else {
                gc.pstdout.print(BN_FALSE, .{}) catch return false;
            }
        } else if (self.isNumber()) {
            const n: f64 = self.asNumber();
            if (math.isInf(n)) {
                gc.pstdout.print(BN_INFINITY, .{}) catch return false;
            } else if (math.isNan(n)) {
                gc.pstdout.print(BN_NAN, .{}) catch return false;
            } else {
                gc.pstdout.print("{d}", .{n}) catch return false;
            }
        } else if (self.isObj()) {
            return self.asObj().printObj(gc, link);
        } else {
            gc.pstdout.print(BN_UNKNOWN, .{}) catch return false;
        }

        return true;
    }

    /// Convert value to string
    /// you must free the result
    pub fn toString(self: Self, al: std.mem.Allocator) ![]u8 {
        if (self.isNil()) {
            const r = try std.fmt.allocPrint(al, BN_NIL, .{});
            return r;
        } else if (self.isBool()) {
            if (self.asBool()) {
                const r = try std.fmt.allocPrint(al, BN_TRUE, .{});
                return r;
            } else {
                const r = try std.fmt.allocPrint(al, BN_FALSE, .{});
                return r;
            }
        } else if (self.isNumber()) {
            var mstr: []u8 = undefined;

            const num = self.asNumber();

            if (math.isInf(num)) {
                mstr = try std.fmt.allocPrint(al, BN_INFINITY, .{});
            } else if (math.isNan(num)) {
                mstr = try std.fmt.allocPrint(al, BN_NAN, .{});
            } else {
                mstr = try std.fmt.allocPrint(al, "{d}", .{self.asNumber()});
            }
            return mstr;
        } else if (self.isObj()) {
            return try self.asObj().toString(al);
        }
        var r = try al.alloc(u8, 1);
        r[0] = '_';
        return r;
    }
};

test "bool values" {
    try std.testing.expect(PValue.makeBool(true).asBool() == true);
    try std.testing.expectEqual(false, PValue.makeBool(!true).asBool());
    try std.testing.expectEqual(false, PValue.makeBool(true).isNumber());
    try std.testing.expectEqual(false, PValue.makeBool(true).isNil());
    try std.testing.expectEqual(false, PValue.makeBool(true).isFalsy());
    try std.testing.expectEqual(true, PValue.makeBool(false).isFalsy());
}

test "number values" {
    const hundred: f64 = 100.0;
    const nnnn: f64 = 99.99;
    try std.testing.expect(
        PValue.makeNumber(@floatCast(100)).asNumber() == hundred,
    );
    try std.testing.expect(
        PValue.makeNumber(@floatCast(99.99)).asNumber() == nnnn,
    );
    try std.testing.expect(PValue.makeNumber(@floatCast(1)).isBool() == false);
    try std.testing.expect(PValue.makeNumber(@floatCast(1)).isNil() == false);
}

test "nil value" {
    try std.testing.expectEqual(PValue.makeNil(), PValue.makeNil());
    try std.testing.expectEqual(true, PValue.makeNil().isNil());
    try std.testing.expectEqual(false, PValue.makeNil().isBool());
    try std.testing.expectEqual(false, PValue.makeNil().isNumber());
}
