//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const PObj = @import("object.zig").PObj;

pub const PValueType = enum(u8) {
    Pt_Num,
    Pt_Bool,
    Pt_Nil,
    Pt_Obj,
    Pt_Unknown,
};

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

    

    /// is value a bool
    pub fn isBool(self : Self) bool {
        return (self.data | 1) == TRUE_VAL.data;
    }
    
    /// is value a nil 
    pub fn isNil(self : Self) bool {
        return self.data == NIL_VAL.data;
    }
    
    /// is value a nil
    pub fn isNumber(self : Self) bool{
        return (self.data & QNAN) != QNAN;
    }

    /// is value a object
    pub fn isObj(self : Self) bool{
        return (self.data & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT);
    }

    pub fn isString(self : Self) bool {
        if (self.isObj()) {
            return self.asObj().isString();
        } 

        return false;
    }
    

    /// get a number value as `f64` 
    pub fn asNumber(self : Self) f64 {
       if (self.isNumber()) {
            return @bitCast(f64, self.data); 
       } else {
            return 0;
       } 
    }


    /// get a bool value as `bool`
    pub fn asBool(self : Self) bool {
        if (self.isBool()) {
            return self.data == TRUE_VAL.data;
        } else {
            return false;
        }
    }

    pub fn asObj(self : Self) *PObj{
        const v : u64 = self.data & ~(SIGN_BIT | QNAN);
        return @intToPtr(*PObj , @intCast(usize, v));
    }


    /// Create a new number value
    pub fn makeNumber(n : f64) PValue {
        return PValue{
            .data = @bitCast(u64, n)
        };
    }


    /// Create a new bool value
    pub fn makeBool(b : bool) PValue{
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

    pub fn makeObj(o : *PObj) PValue{
        return PValue{
            .data = SIGN_BIT | QNAN | @ptrToInt(o),       
        };
    }


    /// Return a new value with with negative value of itself;
    /// If `self` is not a number return itself
    pub fn makeNeg(self : Self) PValue {
        if (self.isNumber()) {
            return PValue.makeNumber(-self.asNumber());
        } else {
            return self;
        }
    }

    pub fn getType(self : Self) PValueType{
        if (self.isNumber()) {
            return .Pt_Num;
        } else if (self.isBool()){
            return .Pt_Bool;
        } else if (self.isNil()){
            return .Pt_Nil;
        } else if (self.isObj()){
            return .Pt_Obj;
        }

        return .Pt_Unknown;
    }

    pub fn isEqual(self : Self , other : PValue) bool {
        if (self.getType() != other.getType()) {
            return false;
        }

        if (self.isBool()) {
            return self.asBool() == other.asBool();
        }else if (self.isNumber()){
            return self.asNumber() == other.asNumber();
        } else if (self.isNil()) {
            return true; 
        } else if (self.isObj()) {
            return true; //object equal test 
        }

        return false;
    }

    /// Chec if value is falsy
    pub fn isFalsy(self : Self) bool{
        if (self.isBool()) { 
            return !self.asBool(); 
        } else if (self.isNil()) {
            return true;
        } else {
            return false;
        }

    }


    /// Print value of PValue to console
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
        } else if (self.isObj()){
           self.asObj().printObj();
        } else{
            std.debug.print("UNKNOWN VALUE", .{});
        }

        
    }

    /// Convert value to string 
    /// you must free the result
    pub fn toString(self : Self , al : std.mem.Allocator) ![]u8{
        if (self.isNil()) {
            var r = try std.fmt.allocPrint(al, "nil" , .{});
            return r;
        } else if (self.isBool()){
            if (self.asBool()) {
                var r = try std.fmt.allocPrint(al, "true", .{});
                return r;
            } else {
                var r = try std.fmt.allocPrint(al, "false", .{}); 
                return r;
            }
        } else if (self.isNumber()) {
            //const size = @intCast( usize , std.fmt.count("{d}", .{self.asNumber()}));
            //const mstr = try al.alloc(u8 , size);
            //_ = try std.fmt.bufPrint(mstr, "{d}", .{self.asNumber()});
            const mstr = try std.fmt.allocPrint(al, "{d}", .{self.asNumber()});
            return mstr;
        } else if (self.isObj()){
            return try self.asObj().toString(al);
        }
            var r = try al.alloc(u8, 1);
            r[0] = '_';
            return r;
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
