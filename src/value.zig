//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

pub const PValue = packed struct {
    data: u64,
    const QNAN: u64 = 0x7ffc000000000000;
    const SIGN_BIT: u64 = 0x8000000000000000;

    const TAG_NIL = 1;
    const TAG_FALSE = 2;
    const TAG_TRUE = 3;
    
    const NIL_VAL = PValue{ .data = QNAN | TAG_NIL };
    const FALSE_VAL = PValue { .data = QNAN | TAG_FALSE };
    const TRUE_VAL = PValue{ .data = QNAN | TAG_TRUE };

    const Self = @This();


    pub fn isBool(self : Self) bool {
        return (self.data | 1) == TRUE_VAL.data;
    }

    pub fn isNil(self : Self) bool {
        return self.data == NIL_VAL.data;
    }

    pub fn isNumber(self : Self) bool{
        return (self.data & QNAN) != QNAN;
    }

    pub fn isObj(self : Self) bool{
        return (self.data & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT);
    }

    pub fn asNumber(self : Self) f64 {
       if (self.isNumber()) {
            return @bitCast(f64, self.data); 
       } else {
            return 0;
       } 
    }

    pub fn asBool(self : Self) bool {
        if (self.isBool()) {
            return self.data == TRUE_VAL.data;
        } else {
            return false;
        }
    }

    pub fn makeNumber(n : f64) PValue {
        return PValue{
            .data = @bitCast(u64, n)
        };
    }

    pub fn makeBool(b : bool) PValue{
        if (b) {
            return TRUE_VAL;
        } else {
            return FALSE_VAL;
        }
    }

    pub fn makeNil() PValue {
        return NIL_VAL;
    }

    pub fn makeNeg(self : Self) PValue {
        if (self.isNumber()) {
            return PValue.makeNumber(-self.asNumber());
        } else {
            return self;
        }
    }

    pub fn isFalsy(self : Self) bool{
        if (self.isBool()) { 
            return !self.asBool(); 
        } else if (self.isNil()) {
            return true;
        } else {
            return false;
        }

    }

    pub fn printVal(self : Self) void{
        if (self.isNil()) {
            std.debug.print("nil", .{});
        } else if (self.isBool()) {
            const b : bool = self.asBool();
            if (b) {
                std.debug.print("true" , .{});
            } else {
                std.debug.print("false" , .{});
            }
        } else if (self.isNumber()){
            const n : f64 = self.asNumber();
            std.debug.print("{d}" , .{n});
        } else{
            std.debug.print("UNKNOWN VALUE", .{});
        }

        
    }



};

test "bool values" {
    try std.testing.expect(PValue.makeBool(true).asBool() == true) ;
    try std.testing.expectEqual(false, PValue.makeBool(!true).asBool());
    try std.testing.expectEqual(false, PValue.makeBool(true).isNumber());
    try std.testing.expectEqual(false, PValue.makeBool(true).isNil());
    try std.testing.expectEqual(false, PValue.makeBool(true).isFalsy());
    try std.testing.expectEqual(true, PValue.makeBool(false).isFalsy());
}

test "number values" {
    try std.testing.expect(PValue.makeNumber(@floatCast(f64, 100)).asNumber() == @floatCast(f64, 100));
    try std.testing.expect(PValue.makeNumber(@floatCast(f64, 99.99)).asNumber() == @floatCast(f64, 99.99));
    try std.testing.expect(PValue.makeNumber(@floatCast(f64, 1)).isBool() == false);
    try std.testing.expect(PValue.makeNumber(@floatCast(f64, 1)).isNil() == false);
}

test "nil value" {
    try std.testing.expectEqual(PValue.makeNil() , PValue.makeNil());
    try std.testing.expectEqual(true, PValue.makeNil().isNil());
    try std.testing.expectEqual(false, PValue.makeNil().isBool());
    try std.testing.expectEqual(false, PValue.makeNil().isNumber());
}
