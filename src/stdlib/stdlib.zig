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
const bnMathMod = @import("bn/bnmath.zig");
const stringMod = @import("string.zig");
const bnStringMod = @import("bn/bnstring.zig");
const mapMod = @import("map.zig");
const bnMapMod = @import("bn/bnmap.zig");
const bigMod = @import("big.zig");
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

pub const OsName = osMod.Name;
pub const BnOsName = bnOsMod.Name;
pub const MathName = mathMod.Name;
pub const BnMathName = bnMathMod.Name;
pub const StringName = stringMod.Name;
pub const BnStringName = bnStringMod.Name;
pub const MapName = mapMod.Name;
pub const BnMapName = bnMapMod.Name;
pub const BigName = bigMod.Name;
pub const BnBigName = bnBigMod.Name;
pub const FileName = fileMod.Name;

pub fn IsStdlib(name: []const u32) bool {
    if (utils.matchU32(name, OsName) or
        utils.matchU32(name, BnOsName) or
        utils.matchU32(name, MathName) or
        utils.matchU32(name, BnMathName) or
        utils.matchU32(name, StringName) or
        utils.matchU32(name, BnStringName) or
        utils.matchU32(name, MapName) or
        utils.matchU32(name, BnMapName) or
        utils.matchU32(name, BigName) or
        utils.matchU32(name, BnBigName) or
        utils.matchU32(name, FileName))
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
    } else if (utils.matchU32(name, BnMathName)) {
        pushStdlibBnMath(v);
        return true;
    } else if (utils.matchU32(name, StringName)) {
        pushStdlibString(v);
        return true;
    } else if (utils.matchU32(name, BnStringName)) {
        pushStdlibBnString(v);
        return true;
    } else if (utils.matchU32(name, MapName)) {
        pushStdlibMap(v);
        return true;
    } else if (utils.matchU32(name, BnMapName)) {
        pushStdlibBnMap(v);
        return true;
    } else if (utils.matchU32(name, BigName)) {
        pushStdlibBig(v);
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
        msl.m(mathMod.NameFuncGcd, mathMod.math_Gcd),
        msl.m(mathMod.NameFuncLcm, mathMod.math_Lcm),
        msl.m(mathMod.NameFuncSine, mathMod.math_Sine),
        msl.m(mathMod.NameFuncCosine, mathMod.math_Cosine),
        msl.m(mathMod.NameFuncTangent, mathMod.math_Tangent),
        msl.m(mathMod.NameFuncDegree, mathMod.math_Degree),
        msl.m(mathMod.NameFuncRadians, mathMod.math_Radians),
        msl.m(mathMod.NameFuncNumber, mathMod.math_Number),
        msl.m(mathMod.NameFuncAbs, mathMod.math_Abs),
        msl.m(mathMod.NameFuncRound, mathMod.math_Round),
        msl.m(mathMod.NameFuncFloor, mathMod.math_Floor),
        msl.m(mathMod.NameFuncCeil, mathMod.math_Ceil),
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

pub fn pushStdlibString(v: *vm.Vm) void {
    _pushStdlib(v, stringMod.Name, &[_]msl{
        msl.m(stringMod.NameFuncSplit, stringMod.str_Split),
        msl.m(stringMod.NameFuncString, stringMod.str_String),
    });
}

pub fn pushStdlibFile(v: *vm.Vm) void {
    _pushStdlib(v, fileMod.Name, &[_]msl{
        msl.m(fileMod.NameFuncRead, fileMod.file_Read),
    });
}

pub fn pushStdlibBnString(v: *vm.Vm) void {
    _pushStdlib(v, bnStringMod.Name, &[_]msl{
        msl.m(bnStringMod.BnNameFuncSplit, bnStringMod.bnstr_Split),
        msl.m(bnStringMod.BnNameFuncString, bnStringMod.bnstr_String),
    });
}

pub fn pushStdlibMap(v: *vm.Vm) void {
    _pushStdlib(v, mapMod.Name, &[_]msl{
        msl.m(mapMod.NameFuncExists, mapMod.map_Exists),
        msl.m(mapMod.NameFuncKeys, mapMod.map_Keys),
        msl.m(mapMod.NameFuncValues, mapMod.map_Values),
    });
}

pub fn pushStdlibBnMap(v: *vm.Vm) void {
    _pushStdlib(v, bnMapMod.Name, &[_]msl{
        msl.m(bnMapMod.BnNameFuncExists, bnMapMod.bnmap_Exists),
        msl.m(bnMapMod.BnNameFuncKeys, bnMapMod.bnmap_Keys),
        msl.m(bnMapMod.BnNameFuncValues, bnMapMod.bnmap_Values),
    });
}

pub fn pushStdlibBig(v: *vm.Vm) void {
    _pushStdlib(v, bigMod.Name, &[_]msl{
        msl.m(bigMod.NameFuncNew, bigMod.big_New),
        msl.m(bigMod.NamefuncAdd, bigMod.big_Add),
        msl.m(bigMod.NamefuncSub, bigMod.big_Sub),
    });
}

pub fn pushStdlibBnBig(v: *vm.Vm) void {
    _pushStdlib(v, bnBigMod.Name, &[_]msl{
        msl.m(bnBigMod.BnNameFuncNew, bnBigMod.bnbig_New),
        msl.m(bnBigMod.BnNamefuncAdd, bnBigMod.bnbig_Add),
        msl.m(bnBigMod.BnNamefuncSub, bnBigMod.bnbig_Sub),
    });
}
