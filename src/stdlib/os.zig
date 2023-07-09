const std = @import("std");
const value = @import("../value.zig");
const Gc = @import("../gc.zig").Gc;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");

pub fn os_Name(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "name() function only takes single argument"
        ).?;
    }

    const nm = switch (builtin.target.os.tag) {
            .windows => "windows",
            .linux => "linux", //should be unix detection instead of linux
            .ios,.macos,.watchos,.tvos => "darwin",
            .kfreebsd , .freebsd , .openbsd , .netbsd , .dragonfly => "bsd",
            .plan9 => "plan9",
            else => {
                if (builtin.target.abi == .android) {
                    "android";
                } else {
                    "unknown";
                }
            },
        };
    
    const v = gc.copyStringU8( nm, 0) orelse {
        return PValue.makeNil();
    };

    return PValue.makeObj(v.parent());
}

pub fn os_Arch(gc : *Gc , argc : u8 , values : []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "arch() function only takes single argument"
        ).?;
    }
    const anm = switch (builtin.target.cpu.arch) {
           .arm,
           .armeb,
           .aarch64,
           .aarch64_be,
           .aarch64_32 => "arm",
           .x86 => "32",
           .x86_64 => "64",
           else => "unknown",
    };

    const v = gc.copyStringU8(anm, 0) orelse {
        return PValue.makeNil();
    };

    return PValue.makeObj(v.parent());
}

pub fn os_Username(gc : *Gc , argc : u8 , values : []PValue) PValue {
      _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "user() function only takes single argument"
        ).?;
    }

    var unm : ?[]const u8 = null;

    if (utils.IS_WIN) {
        unm = std.os.getenv("USERNAME");
    } else if (utils.IS_MAC or utils.IS_LINUX){
        unm = std.os.getenv("USER");
    } else {
        unm = "unknown";
    }
    
    if (unm) |n| {
        const v = gc.copyStringU8(n, 0) orelse return PValue.makeNil();
        return PValue.makeObj(v.parent());
    } else {
        return PValue.makeNil();
    }

}


