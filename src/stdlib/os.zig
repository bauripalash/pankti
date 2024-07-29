//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");
const errs = @import("os_errors.zig");

//ওএস
pub const Name = "ওএস";
// নাম
pub const NameFuncName = "নাম";
pub fn os_Name(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.OS_NAME_ARG_NOT_ZERO,
            .{argc},
        ).?;
    }
    const nm = switch (builtin.target.os.tag) {
        .windows => "উইন্ডোজ",
        .linux => "লিনাক্স", //should be unix detection instead of linux
        .ios => "আইওএস",
        .macos => "ম্যাকওএস",
        .freebsd, .openbsd, .netbsd, .dragonfly => "বিএসডি",
        .plan9 => "প্ল্যান9",
        else => if (builtin.target.abi == .android)
            "আন্ড্রয়েড"
        else if (utils.IS_WASM)
            "ওয়েব"
        else
            "অজানা",
    };

    const obj = vm.gc.copyStringU8(nm, nm.len) catch return PValue.makeNil();
    return PValue.makeObj(obj.parent());
}

pub const NameFuncArch = "আর্চ";
pub fn os_Arch(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.OS_ARCH_ARG_NOT_ZERO,
            .{argc},
        ).?;
    }

    const anm = switch (builtin.target.cpu.arch) {
        .arm, .armeb, .aarch64, .aarch64_be, .aarch64_32 => "আর্ম",
        .x86 => "এক্স86",
        .x86_64 => "এক্স86_64",
        .wasm32, .wasm64 => "ওয়েব",
        else => "অজানা",
    };

    return vm.gc.makeString(anm);
}
pub const NameFuncUsername = "ব্যবহারকারী";
pub fn os_Username(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.OS_USERNAME_ARG_NOT_ZERO,
            .{argc},
        ).?;
    }
    if (utils.IS_WASM) {
        return vm.gc.makeString("ওয়েব");
    }

    var unm: ?[]const u8 = null;

    if (utils.IS_WIN) {
        unm = std.process.getEnvVarOwned(vm.gc.hal(), "USERNAME") catch null;
    } else if (utils.IS_MAC or utils.IS_LINUX) {
        unm = std.process.getEnvVarOwned(vm.gc.hal(), "USER") catch null;
    }

    if (unm) |n| {
        defer vm.gc.hal().free(n);
        return vm.gc.makeString(n);
    } else {
        return vm.gc.makeString("অজানা");
    }
}
pub const NameFuncHomdir = "ঘর";
pub fn os_Homedir(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.OS_HOMEDIR_ARG_NOT_ZERO,
            .{argc},
        ).?;
    }
    if (utils.IS_WASM) {
        return vm.gc.makeString("ওয়েব");
    }
    const hdir: ?[]const u8 = if (utils.IS_WIN)
        std.process.getEnvVarOwned(vm.gc.hal(), "USERPROFILE") catch null
    else if (utils.IS_MAC or utils.IS_LINUX)
        std.process.getEnvVarOwned(vm.gc.hal(), "HOME") catch null
    else
        null;

    if (hdir) |h| {
        defer vm.gc.hal().free(h);
        return vm.gc.makeString(h);
    } else {
        return vm.gc.makeString("অজানা");
    }
}
pub const NameFuncCurdir = "বর্তমান";
pub fn os_Curdir(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.OS_CURDIR_ARG_NOT_ZERO,
            .{argc},
        ).?;
    }
    if (utils.IS_WASM) {
        return vm.gc.makeString("ওয়েব");
    }

    const tempPath = vm.gc.hal().alloc(u8, 1024) catch {
        return vm.gc.makeString("অজানা");
    };

    const dir = std.process.getCwd(tempPath) catch return {
        vm.gc.hal().free(tempPath);
        return vm.gc.makeString("অজানা");
    };

    const result = vm.gc.makeString(dir);

    vm.gc.hal().free(tempPath);

    return result;
}
