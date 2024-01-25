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

pub const Name = &[_]u32{ 'o', 's' };
pub const NameFuncName = &[_]u32{ 'n', 'a', 'm', 'e' };
pub fn os_Name(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;

    if (argc != 0) {
        return PValue.makeError(vm.gc, "name() function only takes single argument").?;
    }

    const nm = switch (builtin.target.os.tag) {
        .windows => "windows",
        .linux => "linux", //should be unix detection instead of linux
        .ios, .macos, .watchos, .tvos => "darwin",
        .kfreebsd, .freebsd, .openbsd, .netbsd, .dragonfly => "bsd",
        .plan9 => "plan9",
        else => if (builtin.target.abi == .android) "android" else if (utils.IS_WASM) "wasm" else "unknown",
    };

    return vm.gc.makeString(nm);
}

pub const ArchFuncName = &[_]u32{ 'a', 'r', 'c', 'h' };
pub fn os_Arch(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(vm.gc, "arch() function only takes single argument").?;
    }
    const anm = switch (builtin.target.cpu.arch) {
        .arm, .armeb, .aarch64, .aarch64_be, .aarch64_32 => "arm",
        .x86 => "32",
        .x86_64 => "64",
        .wasm32, .wasm64 => "wasm",
        else => "unknown",
    };

    return vm.gc.makeString(anm);
}

pub const UsernameFuncName = &[_]u32{ 'u', 's', 'e', 'r' };
pub fn os_Username(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(vm.gc, "user() function only takes single argument").?;
    }

    if (utils.IS_WASM) {
        return vm.gc.makeString("wasm");
    }

    var unm: ?[]const u8 = null;

    if (utils.IS_WIN) {
        unm = std.os.getenv("USERNAME");
    } else if (utils.IS_MAC or utils.IS_LINUX) {
        unm = std.os.getenv("USER");
    } else {
        unm = "unknown";
    }

    if (unm) |n| {
        return vm.gc.makeString(n);
    } else {
        return vm.gc.makeString("unknown");
    }
}

pub const HomedirFuncName = &[_]u32{ 'h', 'o', 'm', 'e' };
pub fn os_Homerdir(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(vm.gc, "home() function only takes single argument").?;
    }

    if (utils.IS_WASM) {
        return vm.gc.makeString("wasm");
    }
    const hdir: ?[]const u8 = if (utils.IS_WIN)
        std.os.getenv("USERPROFILE")
    else if (utils.IS_MAC or utils.IS_LINUX)
        std.os.getenv("HOME")
    else
        "unknown";

    if (hdir) |h| {
        return vm.gc.makeString(h);
    } else {
        return vm.gc.makeString("unknown");
    }
}

pub const CurdirFuncName = &[_]u32{ 'c', 'u', 'r', 'd', 'i', 'r' };
pub fn os_Curdir(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(vm.gc, "home() function only takes single argument").?;
    }

    if (utils.IS_WASM) {
        return vm.gc.makeString("wasm");
    }

    const tempPath = vm.gc.hal().alloc(u8, 1024) catch {
        return vm.gc.makeString("unknown");
    };

    const dir = std.os.getcwd(tempPath) catch return {
        vm.gc.hal().free(tempPath);
        return vm.gc.makeString("unknown");
    };

    const result = vm.gc.makeString(dir);

    vm.gc.hal().free(tempPath);

    return result;
}
