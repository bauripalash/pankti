const std = @import("std");
const value = @import("../value.zig");
const Gc = @import("../gc.zig").Gc;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");

pub const Name = &[_]u32{'o' , 's'};
pub const NameFuncName = &[_]u32{ 'n' , 'a' , 'm' , 'e' };
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
            else => if (builtin.target.abi == .android) "android" else if (utils.IS_WASM) "wasm" else "unknown",
        };
    
    return gc.makeString(nm);
}

pub const ArchFuncName = &[_]u32{ 'a' , 'r' , 'c' , 'h' };
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
           .wasm32, .wasm64 => "wasm",
           else => "unknown",
    };

    return gc.makeString(anm);
}


pub const UsernameFuncName = &[_]u32{ 'u' , 's' , 'e' , 'r' };
pub fn os_Username(gc : *Gc , argc : u8 , values : []PValue) PValue {
      _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "user() function only takes single argument"
        ).?;
    }

    if (utils.IS_WASM) {
        return gc.makeString("wasm");
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
        return gc.makeString(n);
    } else {
        return gc.makeString("unknown");
    }

}



pub const HomedirFuncName = &[_]u32{ 'h' , 'o' , 'm' , 'e' };
pub fn os_Homerdir(gc : *Gc , argc : u8 , values : []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "home() function only takes single argument"
        ).?;
    }

    if (utils.IS_WASM) { return gc.makeString("wasm"); }
    var hdir : ?[]const u8 = if (utils.IS_WIN) 
        std.os.getenv("USERPROFILE")
    else if (utils.IS_MAC or utils.IS_LINUX) 
        std.os.getenv("HOME")
    else 
        "unknown";
    
    if (hdir) |h| {
        return gc.makeString(h);
    } else {
        return gc.makeString("unknown");
    }

}

pub const CurdirFuncName = &[_]u32{ 'c' , 'u' , 'r' , 'd' , 'i' , 'r' };
pub fn os_Curdir(gc : *Gc , argc : u8 , values : []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc, 
            "home() function only takes single argument"
        ).?;
    }

    if (utils.IS_WASM) { return gc.makeString("wasm"); }

    var tempPath = gc.hal().alloc(u8, 1024) catch {
        return gc.makeString("unknown");
    };

    const dir = std.os.getcwd(tempPath) catch return {
        gc.hal().free(tempPath);
        return gc.makeString("unknown");
    };
    
    const result = gc.makeString(dir);

    gc.hal().free(tempPath);

    return result;
}

