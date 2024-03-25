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

const bnOsMod = @import("bn/bnos.zig");
const bnMathMod = @import("bn/bnmath.zig");
const bnStringMod = @import("bn/bnstring.zig");
const bnMapMod = @import("bn/bnmap.zig");
const bnBigMod = @import("bn/bnbig.zig");
const fileMod = @import("file.zig");

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

pub const BnOsName = bnOsMod.Name;
pub const BnMathName = bnMathMod.Name;
pub const BnStringName = bnStringMod.Name;
pub const BnMapName = bnMapMod.Name;
pub const BnBigName = bnBigMod.Name;
pub const FileName = fileMod.Name;

pub fn IsStdlib(name: []const u32) bool {
    if (utils.matchU32(name, BnOsName) or
        utils.matchU32(name, BnMathName) or
        utils.matchU32(name, BnStringName) or
        utils.matchU32(name, BnMapName) or
        utils.matchU32(name, BnBigName) or
        utils.matchU32(name, FileName))
    {
        return true;
    }

    return false;
}

pub fn PushStdlib(v: *vm.Vm, name: []const u32) bool {
    if (utils.matchU32(name, BnOsName)) {
        pushStdlibBnOs(v);
        return true;
    } else if (utils.matchU32(name, BnMathName)) {
        pushStdlibBnMath(v);
        return true;
    } else if (utils.matchU32(name, BnStringName)) {
        pushStdlibBnString(v);
        return true;
    } else if (utils.matchU32(name, BnMapName)) {
        pushStdlibBnMap(v);
        return true;
    } else if (utils.matchU32(name, BnBigName)) {
        pushStdlibBnBig(v);
        return true;
    } else if (utils.matchU32(name, FileName)) {
        pushStdlibFile(v);
        return true;
    }

    return false;
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

pub fn pushStdlibBnMath(v: *vm.Vm) void {
    _pushStdlib(v, bnMathMod.Name, &[_]msl{
        msl.m(bnMathMod.BnNameFuncPi, bnMathMod.bnmath_Pi),
        msl.m(bnMathMod.BnNameFuncE, bnMathMod.bnmath_E),
        msl.m(bnMathMod.BnNameFuncSqrt, bnMathMod.bnmath_Sqrt),
        msl.m(bnMathMod.BnNameFuncLog10, bnMathMod.bnmath_Log10),
        msl.m(bnMathMod.BnNameFuncLog, bnMathMod.bnmath_Log),
        msl.m(bnMathMod.BnNameFuncLogX, bnMathMod.bnmath_LogX),
        msl.m(bnMathMod.BnNameFuncGcd, bnMathMod.bnmath_Gcd),
        msl.m(bnMathMod.BnNameFuncLcm, bnMathMod.bnmath_Lcm),
        msl.m(bnMathMod.BnNameFuncSine, bnMathMod.bnmath_Sine),
        msl.m(bnMathMod.BnNameFuncCosine, bnMathMod.bnmath_Cosine),
        msl.m(bnMathMod.BnNameFuncTangent, bnMathMod.bnmath_Tangent),
        msl.m(bnMathMod.BnNameFuncDegree, bnMathMod.bnmath_Degree),
        msl.m(bnMathMod.BnNameFuncRadians, bnMathMod.bnmath_Radians),
        msl.m(bnMathMod.BnNameFuncNumber, bnMathMod.bnmath_Number),
        msl.m(bnMathMod.BnNameFuncAbs, bnMathMod.bnmath_Abs),
        msl.m(bnMathMod.BnNameFuncRound, bnMathMod.bnmath_Round),
        msl.m(bnMathMod.BnNameFuncFloor, bnMathMod.bnmath_Floor),
        msl.m(bnMathMod.BnNameFuncCeil, bnMathMod.bnmath_Ceil),
    });
}

pub fn pushStdlibFile(v: *vm.Vm) void {
    _pushStdlib(v, fileMod.Name, &[_]msl{
        msl.m(fileMod.NameFuncRead, fileMod.file_Read),
        msl.m(fileMod.NameFuncWrite, fileMod.file_Write),
    });
}

pub fn pushStdlibBnString(v: *vm.Vm) void {
    _pushStdlib(v, bnStringMod.Name, &[_]msl{
        msl.m(bnStringMod.BnNameFuncSplit, bnStringMod.bnstr_Split),
        msl.m(bnStringMod.BnNameFuncString, bnStringMod.bnstr_String),
    });
}

pub fn pushStdlibBnMap(v: *vm.Vm) void {
    _pushStdlib(v, bnMapMod.Name, &[_]msl{
        msl.m(bnMapMod.BnNameFuncExists, bnMapMod.bnmap_Exists),
        msl.m(bnMapMod.BnNameFuncKeys, bnMapMod.bnmap_Keys),
        msl.m(bnMapMod.BnNameFuncValues, bnMapMod.bnmap_Values),
    });
}

pub fn pushStdlibBnBig(v: *vm.Vm) void {
    _pushStdlib(v, bnBigMod.Name, &[_]msl{
        msl.m(bnBigMod.BnNameFuncNew, bnBigMod.bnbig_New),
        msl.m(bnBigMod.BnNamefuncAdd, bnBigMod.bnbig_Add),
        msl.m(bnBigMod.BnNamefuncSub, bnBigMod.bnbig_Sub),
    });
}
