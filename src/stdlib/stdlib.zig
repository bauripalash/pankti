//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

const vm = @import("../vm.zig");
const utils = @import("../utils.zig");
const gc = @import("../gc.zig");
const table = @import("../table.zig");
const PObj = @import("../object.zig").PObj;

const osMod = @import("os.zig");
const bnOsMod = @import("bn/bnos.zig");
const mathMod = @import("math.zig");

pub const msl = struct {
    key: []const u32,
    func: PObj.ONativeFunction.NativeFn,

    pub fn m(key: []const u32, func: PObj.ONativeFunction.NativeFn) msl {
        return msl{
            .key = key,
            .func = func,
        };
    }
};

pub fn _addStdlib(
    v: *vm.Vm,
    tab: *table.PankTable(),
    name: []const u32,
    func: PObj.ONativeFunction.NativeFn,
) !void {
    const nstr = try v.gc.copyString(name, @truncate(name.len));

    try v.stack.push(nstr.parent().asValue());
    const nf = v.gc.newNative(v, func) orelse return;
    try v.stack.push(nf);

    try tab.put(
        v.gc.hal(),
        v.peek(1).asObj().asString(),
        v.peek(0),
    );

    _ = try v.stack.pop();
    _ = try v.stack.pop();
}

fn _pushStdlib(v: *vm.Vm, modname: []const u32, items: []const msl) void {
    const nameHash = utils.hashU32(modname, v.gc) catch return;

    //std.debug.print("HASH AT PUSH->{d}\n" , .{nameHash});
    v.gc.stdlibs[v.gc.stdlibCount] = gc.StdLibMod.new();
    v.gc.stdlibs[v.gc.stdlibCount].name = modname;
    v.gc.stdlibs[v.gc.stdlibCount].hash = nameHash;
    v.gc.stdlibs[v.gc.stdlibCount].ownerCount = 0;

    v.gc.stdlibCount += 1;
    var i: usize = 0;

    while (i < items.len) : (i += 1) {
        _addStdlib(
            v,
            &v.gc.stdlibs[v.gc.stdlibCount - 1].items,
            items[i].key,
            items[i].func,
        ) catch {
            return;
        };
    }
}

pub const OsName = osMod.Name;
pub const BnOsName = bnOsMod.Name;
pub const MathName = mathMod.Name;

pub fn IsStdlib(name: []const u32) bool {
    if (utils.matchU32(name, OsName) or
        utils.matchU32(name, BnOsName) or
        utils.matchU32(name, MathName))
    {
        return true;
    }

    return false;
}

pub fn PushStdlib(v: *vm.Vm, name: []const u32) bool {
    if (utils.matchU32(name, OsName)) {
        pushStdlibOs(v);
        return true;
    } else if (utils.matchU32(name, BnOsName)) {
        pushStdlibBnOs(v);
        return true;
    } else if (utils.matchU32(name, MathName)) {
        pushStdlibMath(v);
        return true;
    }

    return false;
}

pub fn pushStdlibOs(v: *vm.Vm) void {
    _pushStdlib(v, osMod.Name, &[_]msl{
        msl.m(osMod.NameFuncName, osMod.os_Name),
        msl.m(osMod.ArchFuncName, osMod.os_Arch),
        msl.m(osMod.UsernameFuncName, osMod.os_Username),
        msl.m(osMod.HomedirFuncName, osMod.os_Homerdir),
        msl.m(osMod.CurdirFuncName, osMod.os_Curdir),
    });
}

pub fn pushStdlibBnOs(v: *vm.Vm) void {
    _pushStdlib(v, bnOsMod.Name, &[_]msl{
        msl.m(bnOsMod.BnNameFuncName, bnOsMod.bnos_Name),
        msl.m(bnOsMod.BnNameFuncArch, bnOsMod.bnos_Arch),
        msl.m(bnOsMod.BnNameFuncUsername, bnOsMod.bnos_Username),
        msl.m(bnOsMod.BnNameFuncHomdir, bnOsMod.bnos_Homedir),
        msl.m(bnOsMod.BnNameFuncCurdir, bnOsMod.bnos_Curdir),
    });
}

pub fn pushStdlibMath(v: *vm.Vm) void {
    _pushStdlib(v, mathMod.Name, &[_]msl{
        msl.m(mathMod.NameFuncPi, mathMod.math_Pi),
        msl.m(mathMod.NameFuncE, mathMod.math_E),
        msl.m(mathMod.NameFuncSqrt, mathMod.math_Sqrt),
        msl.m(mathMod.NameFuncLog10, mathMod.math_Log10),
        msl.m(mathMod.NameFuncLog, mathMod.math_Log),
        msl.m(mathMod.NameFuncLogX, mathMod.math_LogX),
    });
}
