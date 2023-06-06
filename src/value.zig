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
        return (self.data & 1) == TRUE_VAL.data;
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

    pub fn isFalsy(self : Self) bool{
        if (self.isBool()) { 
            return !self.asBool(); 
        } else if (self.isNil()) {
            return true;
        } else {
            return false;
        }

    }



};
