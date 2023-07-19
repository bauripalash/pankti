//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("../../value.zig");
const Vm = @import("../../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../../utils.zig");
const stdlib = @import("../stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");
const enmod = @import("../math.zig");

//গণিত
pub const Name = &[_]u32{ 0x0997, 0x09a3, 0x09bf, 0x09a4 };
pub const BnNameFuncPi = &[_]u32{ 0x09aa, 0x09be, 0x0987 };
pub fn bnmath_Pi(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "পাই() কাজটি মাত্র একটি চলরাশি গ্রহণ করে!",
        ).?;
    }

    return enmod.math_Pi(vm, argc, values);
}

pub const BnNameFuncE = &[_]u32{0x0987};
pub fn bnmath_E(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "ই() কাজটি মাত্র একটি চলরাশি গ্রহণ করে!",
        ).?;
    }

    return enmod.math_E(vm, argc, values);
}

pub const BnNameFuncSqrt = &[_]u32{ 0x09ac, 0x09b0, 0x09cd, 0x0997, 0x09ae, 0x09c2, 0x09b2 };
pub fn bnmath_Sqrt(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Sqrt(vm, argc, values);
}

pub const BnNameFuncLog10 = &[_]u32{ 0x09b2, 0x0997, 0x09a6, 0x09b6 };
pub fn bnmath_Log10(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Log10(vm, argc, values);
}

pub const BnNameFuncLog = &[_]u32{
    0x09b2,
    0x0997,
};
pub fn bnmath_Log(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Log(vm, argc, values);
}

pub const BnNameFuncLogX = &[_]u32{ 0x09b2, 0x0997, 0x09ac, 0x09c7, 0x09b8 };
pub fn bnmath_LogX(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_LogX(vm, argc, values);
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

pub const BnNameFuncGcd = &[_]u32{ 0x0997, 0x09b8, 0x09be, 0x0997, 0x09c1 };
pub fn bnmath_Gcd(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Gcd(vm, argc, values);
}

pub const BnNameFuncLcm = &[_]u32{ 0x09b2, 0x09b8, 0x09be, 0x0997, 0x09c1 };
pub fn bnmath_Lcm(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Lcm(vm, argc, values);
}

pub const BnNameFuncSine = &[_]u32{ 0x09b8, 0x09be, 0x0987, 0x09a8 };
pub fn bnmath_Sine(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Sine(vm, argc, values);
}

pub const BnNameFuncCosine = &[_]u32{ 0x0995, 0x09b8 };
pub fn bnmath_Cosine(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Cosine(vm, argc, values);
}

pub const BnNameFuncTangent = &[_]u32{ 0x099f, 0x09cd, 0x09af, 0x09be, 0x09a8 };
pub fn bnmath_Tangent(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Tangent(vm, argc, values);
}

pub const BnNameFuncDegree = &[_]u32{ 0x09a1, 0x09bf, 0x0997, 0x09cd, 0x09b0, 0x09bf };
pub fn bnmath_Degree(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Degree(vm, argc, values);
}

pub const BnNameFuncRadians = &[_]u32{ 0x09b0, 0x09c7, 0x09a1, 0x09bf, 0x09df, 0x09be, 0x09a8 };
pub fn bnmath_Radians(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Radians(vm, argc, values);
}

pub const BnNameFuncNumber = &[_]u32{ 0x09b8, 0x0982, 0x0996, 0x09cd, 0x09af, 0x09be };
pub fn bnmath_Number(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Number(vm, argc, values);
}

pub const BnNameFuncAbs = &[_]u32{ 0x09aa, 0x09b0, 0x09ae };
pub fn bnmath_Abs(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Abs(vm, argc, values);
}

pub const BnNameFuncRound = &[_]u32{ 0x09b0, 0x09be, 0x0989, 0x09a8, 0x09cd, 0x09a1 };
pub fn bnmath_Round(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Round(vm, argc, values);
}

pub const BnNameFuncFloor = &[_]u32{ 0x09ab, 0x09cd, 0x09b2, 0x09cb, 0x09b0 };
pub fn bnmath_Floor(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Floor(vm, argc, values);
}

pub const BnNameFuncCeil = &[_]u32{ 0x09b8, 0x09bf, 0x09b2 };
pub fn bnmath_Ceil(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enmod.math_Ceil(vm, argc, values);
}
