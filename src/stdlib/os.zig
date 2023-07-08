const std = @import("std");
const value = @import("../value.zig");
const Gc = @import("../gc.zig").Gc;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const msl = stdlib.msl;


pub fn os_Name(gc : *Gc , argc: u8, values: []PValue) PValue {
    _ = values;
    _ = argc;
    const v =
        gc.copyStringU8("linux", 5) orelse {
            return PValue.makeNil();
        };
    return PValue.makeObj(v.parent());
}

pub fn _get_os() []msl{
    const items = [_]stdlib.msl{
       msl.m(&[_]u32{'n' , 'a' , 'm' , 'e'}, os_Name),
    };

    return &items;
}

