const std = @import("std");
const value = @import("../../value.zig");
const Gc = @import("../../gc.zig").Gc;
const PValue = value.PValue;
const utils = @import("../../utils.zig");
const stdlib = @import("../stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");

pub const Name = &[_]u32{0x0993,0x098f,0x09b8 };
// নাম
pub const BnNameFuncName = &[_]u32{ 0x09a8, 0x09be, 0x09ae };
pub fn bnos_Name(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            gc,
            "ওএস -এর নাম() কাজটি কোনো চলরাশি গ্রহণ করে না",
        ).?;
    }
    const nm = switch (builtin.target.os.tag) {
        .windows => "উইন্ডোজ",
        .linux => "লিনাক্স", //should be unix detection instead of linux
        .ios, .macos, .watchos, .tvos => "ডারউইন",
        .kfreebsd, .freebsd, .openbsd, .netbsd, .dragonfly => "বিএসডি",
        .plan9 => "প্ল্যান9",
        else => if (builtin.target.abi == .android)
            "আন্ড্রয়েড"
        else if (utils.IS_WASM)
            "ওয়েব"
        else
            "অজানা",
    };

    return gc.makeString(nm);
}

pub const BnNameFuncArch = &[_]u32{ 0x0986, 0x09b0, 0x09cd, 0x099a };
pub fn bnos_Arch(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(gc, "ওএস -এর আর্চ() কাজটি কোনো চলরাশি গ্রহণ করে না").?;
    }

    const anm = switch (builtin.target.cpu.arch) {
        .arm, .armeb, .aarch64, .aarch64_be, .aarch64_32 => "arm",
        .x86 => "32",
        .x86_64 => "64",
        .wasm32, .wasm64 => "wasm",
        else => "অজানা",
    };

    return gc.makeString(anm);
}
pub const BnNameFuncUsername = &[_]u32{ 0x09ac, 0x09cd, 0x09af, 0x09ac, 0x09b9, 0x09be, 0x09b0, 0x0995, 0x09be, 0x09b0, 0x09c0 };
pub fn bnos_Username(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(gc, "ওএস -এর ব্যবহারকারী() কাজটি কোনো চলরাশি গ্রহণ করে না").?;
    }
    if (utils.IS_WASM) {
        return gc.makeString("wasm");
    }

    var unm: ?[]const u8 = null;

    if (utils.IS_WIN) {
        unm = std.os.getenv("USERNAME");
    } else if (utils.IS_MAC or utils.IS_LINUX) {
        unm = std.os.getenv("USER");
    } else {
        unm = "অজানা";
    }

    if (unm) |n| {
        return gc.makeString(n);
    } else {
        return gc.makeString("অজানা");
    }
}
pub const BnNameFuncHomdir = &[_]u32{ 0x0998, 0x09b0 };
pub fn bnos_Homedir(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(gc, "ওএস -এর ঘর() কাজটি কোনো চলরাশি গ্রহণ করে না").?;
    }
    if (utils.IS_WASM) {
        return gc.makeString("wasm");
    }
    var hdir: ?[]const u8 = if (utils.IS_WIN)
        std.os.getenv("USERPROFILE")
    else if (utils.IS_MAC or utils.IS_LINUX)
        std.os.getenv("HOME")
    else
        "অজানা";

    if (hdir) |h| {
        return gc.makeString(h);
    } else {
        return gc.makeString("অজানা");
    }
}
pub const BnNameFuncCurdir = &[_]u32{ 0x09ac, 0x09b0, 0x09cd, 0x09a4, 0x09ae, 0x09be, 0x09a8 };
pub fn bnos_Curdir(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(gc, "ওএস -এর বর্তমান() কাজটি কোনো চলরাশি গ্রহণ করে না").?;
    }
    if (utils.IS_WASM) {
        return gc.makeString("wasm");
    }

    var tempPath = gc.hal().alloc(u8, 1024) catch {
        return gc.makeString("অজানা");
    };

    const dir = std.os.getcwd(tempPath) catch return {
        gc.hal().free(tempPath);
        return gc.makeString("অজানা");
    };

    const result = gc.makeString(dir);

    gc.hal().free(tempPath);

    return result;
}
