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

const CONST_PI: f64 = 3.14159265358979323846;
const CONST_E: f64 = 2.71828182845904523536;

//গণিত
pub const Name = &[_]u32{ 0x0997, 0x09a3, 0x09bf, 0x09a4 };
pub const NameFuncPi = &[_]u32{ 0x09aa, 0x09be, 0x0987 };
pub fn math_Pi(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "পাই() কাজটি মাত্র একটি চলরাশি গ্রহণ করে!",
        ).?;
    }

    return PValue.makeNumber(CONST_PI);
}

pub const NameFuncE = &[_]u32{0x0987};
pub fn math_E(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;
    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "ই() কাজটি মাত্র একটি চলরাশি গ্রহণ করে!",
        ).?;
    }

    return PValue.makeNumber(CONST_E);
}

pub const NameFuncSqrt = &[_]u32{ 0x09ac, 0x09b0, 0x09cd, 0x0997, 0x09ae, 0x09c2, 0x09b2 };
pub fn math_Sqrt(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "বর্গমূল() কাজটি মাত্র একটি চলরাশি গ্রহণ করে!",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের বর্গমূল(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.sqrt(rawValue.asNumber()));
}

pub const NameFuncLog10 = &[_]u32{ 0x09b2, 0x0997, 0x09a6, 0x09b6 };
pub fn math_Log10(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "লগদশ() কাজটি  শুধু একটি চলরাশি গ্রহণ",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের লগদশ(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.log10(rawValue.asNumber()));
}

pub const NameFuncLog = &[_]u32{ 0x09b2, 0x0997 };
pub fn math_Log(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "লগ() কাজটি  শুধু একটি চলরাশি গ্রহণ",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের লগ(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(@log(rawValue.asNumber()));
}

pub const NameFuncLogX = &[_]u32{ 0x09b2, 0x0997, 0x09ac, 0x09c7, 0x09b8 };
pub fn math_LogX(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "দগদশ() কাজটি শুধু দুটি চলরাশি গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের লগবেস(ক , খ) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    const rawBase = values[0].asNumber();
    const rawNum = values[1].asNumber();

    return PValue.makeNumber(std.math.log(f64, rawBase, rawNum));
}

pub fn getGcd(a: f64, b: f64) f64 {
    var x = if (a > 0) a else -a;
    var y = if (b > 0) b else -b;

    while (x != y) {
        if (x > y) {
            x -= y;
        } else {
            y -= x;
        }
    }

    return x;
}

pub const NameFuncGcd = &[_]u32{ 0x0997, 0x09b8, 0x09be, 0x0997, 0x09c1 };
pub fn math_Gcd(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "গসাগু() দুটি মাত্র চলরাশি গ্রহণ করে।",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের গসাগু(ক , খ) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(getGcd(
        values[0].asNumber(),
        values[1].asNumber(),
    ));
}

pub const NameFuncLcm = &[_]u32{ 0x09b2, 0x09b8, 0x09be, 0x0997, 0x09c1 };
pub fn math_Lcm(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "গণিতের লসাগু(ক , খ) কাজটি শুধু দুটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের লসাগু(ক , খ) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    const a = values[0].asNumber();
    const b = values[1].asNumber();

    return PValue.makeNumber((a * b) / getGcd(a, b));
}

pub const NameFuncSine = &[_]u32{ 0x09b8, 0x09be, 0x0987, 0x09a8 };
pub fn math_Sine(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "সাইন(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের সাইন(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.sin(values[0].asNumber()));
}

pub const NameFuncCosine = &[_]u32{ 0x0995, 0x09b8 };
pub fn math_Cosine(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "কস(ক) কাজটি শুধু একটি মান গ্রহণ করে ",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের কস(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.cos(values[0].asNumber()));
}

pub const NameFuncTangent = &[_]u32{ 0x099f, 0x09cd, 0x09af, 0x09be, 0x09a8 };
pub fn math_Tangent(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "ট্যান(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের ট্যান(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.tan(values[0].asNumber()));
}

pub const NameFuncDegree = &[_]u32{ 0x09a1, 0x09bf, 0x0997, 0x09cd, 0x09b0, 0x09bf };
pub fn math_Degree(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "ডিগ্রি(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের ডিগ্রি(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(values[0].asNumber() * (180 / CONST_PI));
}

pub const NameFuncRadians = &[_]u32{ 0x09b0, 0x09c7, 0x09a1, 0x09bf, 0x09df, 0x09be, 0x09a8 };
pub fn math_Radians(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "রেডিয়ান(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের রেডিয়ান(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(values[0].asNumber() * (CONST_PI / 180));
}

pub const NameFuncNumber = &[_]u32{ 0x09b8, 0x0982, 0x0996, 0x09cd, 0x09af, 0x09be };
pub fn math_Number(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "সংখ্যা(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isString()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের সংখ্যা(ক) কাজটি শুধুমাত্র স্ট্রিং/নাম মান গ্রহণ করে",
        ).?;
    }

    const stringU8 = utils.u32tou8(
        values[0].asObj().asString().chars,
        vm.gc.hal(),
    ) catch return PValue.makeNumber(0);
    const rawNum: f64 = std.fmt.parseFloat(f64, stringU8) catch 0;

    const result = PValue.makeNumber(rawNum);

    vm.gc.hal().free(stringU8);

    return result;
}

pub const NameFuncAbs = &[_]u32{ 0x09aa, 0x09b0, 0x09ae };
pub fn math_Abs(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "পরম(ক) কাজটি একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের পরম(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(@abs(values[0].asNumber()));
}

pub const NameFuncRound = &[_]u32{ 0x09b0, 0x09be, 0x0989, 0x09a8, 0x09cd, 0x09a1 };
pub fn math_Round(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "রাউন্ড(ক) কাজটি শুধু একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের রাউন্ড(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.round(values[0].asNumber()));
}

pub const NameFuncFloor = &[_]u32{ 0x09ab, 0x09cd, 0x09b2, 0x09cb, 0x09b0 };
pub fn math_Floor(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "ফ্লোর(ক) কাজটি একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের ফ্লোর(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.floor(values[0].asNumber()));
}

pub const NameFuncCeil = &[_]u32{ 0x09b8, 0x09bf, 0x09b2 };
pub fn math_Ceil(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "সিল(ক) কাজটি একটি মান গ্রহণ করে",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "গণিতের সিল(ক) কাজটি শুধু সংখ্যা মান গ্রহণ করে",
        ).?;
    }

    return PValue.makeNumber(std.math.ceil(values[0].asNumber()));
}
