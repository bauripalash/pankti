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
const mathMod = @import("math.zig");
const stringMod = @import("string.zig");
const mapMod = @import("map.zig");
const bigMod = @import("big.zig");
//const fileMod = @import("file.zig");

pub const msl = struct {
    key: []const u8,
    func: PObj.ONativeFunction.NativeFn,

    pub fn m(key: []const u8, func: PObj.ONativeFunction.NativeFn) msl {
        return msl{
            .key = key,
            .func = func,
        };
    }
};

pub fn _addStdlib(
    v: *vm.Vm,
    tab: *table.PankTable(),
    name: []const u8,
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

fn _pushStdlib(v: *vm.Vm, modname: []const u8, items: []const msl) void {
    const nameHash = utils.hashChars(modname, v.gc) catch return;

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
pub const MathName = mathMod.Name;
pub const StringName = stringMod.Name;
pub const MapName = mapMod.Name;
pub const BigName = bigMod.Name;
//pub const FileName = fileMod.Name;

pub fn IsStdlib(name: []const u8) bool {
    if (utils.matchU8(name, OsName) or
        utils.matchU8(name, MathName) or
        utils.matchU8(name, StringName) or
        utils.matchU8(name, MapName) or
        utils.matchU8(name, BigName))
    //utils.matchU8(name, FileName))
    {
        return true;
    }

    return false;
}

pub fn PushStdlib(v: *vm.Vm, name: []const u8) bool {
    if (utils.matchU8(name, OsName)) {
        pushStdlibOs(v);
        return true;
    } else if (utils.matchU8(name, MathName)) {
        pushStdlibMath(v);
        return true;
    } else if (utils.matchU8(name, StringName)) {
        pushStdlibString(v);
        return true;
    } else if (utils.matchU8(name, MapName)) {
        pushStdlibMap(v);
        return true;
    } else if (utils.matchU8(name, BigName)) {
        pushStdlibBig(v);
        return true;
        // } else if (utils.matchU8(name, FileName)) {
        //     pushStdlibFile(v);
        //     return true;
        // }
    }

    return false;
}

pub fn pushStdlibOs(v: *vm.Vm) void {
    _pushStdlib(v, osMod.Name, &[_]msl{
        msl.m(osMod.NameFuncName, osMod.os_Name),
        msl.m(osMod.NameFuncArch, osMod.os_Arch),
        msl.m(osMod.NameFuncUsername, osMod.os_Username),
        msl.m(osMod.NameFuncHomdir, osMod.os_Homedir),
        msl.m(osMod.NameFuncCurdir, osMod.os_Curdir),
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

pub fn pushStdlibString(v: *vm.Vm) void {
    _pushStdlib(v, stringMod.Name, &[_]msl{
        msl.m(stringMod.NameFuncSplit, stringMod.str_Split),
        msl.m(stringMod.NameFuncString, stringMod.str_String),
        msl.m(stringMod.NameFuncUnicode, stringMod.str_Unicode),
    });
}

// pub fn pushStdlibFile(v: *vm.Vm) void {
//     _pushStdlib(v, fileMod.Name, &[_]msl{
//         msl.m(fileMod.NameFuncRead, fileMod.file_Read),
//         msl.m(fileMod.NameFuncWrite, fileMod.file_Write),
//     });
// }

pub fn pushStdlibMap(v: *vm.Vm) void {
    _pushStdlib(v, mapMod.Name, &[_]msl{
        msl.m(mapMod.NameFuncExists, mapMod.map_Exists),
        msl.m(mapMod.NameFuncKeys, mapMod.map_Keys),
        msl.m(mapMod.NameFuncValues, mapMod.map_Values),
    });
}

pub fn pushStdlibBig(v: *vm.Vm) void {
    _pushStdlib(v, bigMod.Name, &[_]msl{
        msl.m(bigMod.NameFuncNew, bigMod.big_New),
        msl.m(bigMod.NamefuncAdd, bigMod.big_Add),
        msl.m(bigMod.NamefuncSub, bigMod.big_Sub),
        msl.m(bigMod.NamefuncDiv, bigMod.big_Div),
        msl.m(bigMod.NamefuncMul, bigMod.big_Mul),
        msl.m(bigMod.NamefuncSqrt, bigMod.big_Sqrt),
    });
}
