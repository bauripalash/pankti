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

pub const Name = &[_]u32{ 'm', 'a', 't', 'h' };

pub const NameFuncPi = &[_]u32{ 'p', 'i' };
pub fn math_Pi(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;

    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "pi() function takes no argument",
        ).?;
    }

    return PValue.makeNumber(CONST_PI);
}

pub const NameFuncE = &[_]u32{'e'};
pub fn math_E(vm: *Vm, argc: u8, values: []PValue) PValue {
    _ = values;

    if (argc != 0) {
        return PValue.makeError(
            vm.gc,
            "e() function takes no argument",
        ).?;
    }

    return PValue.makeNumber(CONST_E);
}

pub const NameFuncSqrt = &[_]u32{ 's', 'q', 'r', 't' };
pub fn math_Sqrt(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "sqrt() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "sqrt() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(std.math.sqrt(rawValue.asNumber()));
}

pub const NameFuncLog10 = &[_]u32{ 'l', 'o', 'g', '1', '0' };
pub fn math_Log10(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "log10() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "log10() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(std.math.log10(rawValue.asNumber()));
}

pub const NameFuncLog = &[_]u32{ 'l', 'o', 'g' };
pub fn math_Log(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "log() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "log() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(@log(rawValue.asNumber()));
}

pub const NameFuncLogX = &[_]u32{ 'l', 'o', 'g', 'x' };
pub fn math_LogX(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "logx() function only takes 2 argument",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "logx(a,b) requires both argument to be numbers",
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

pub const NameFuncGcd = &[_]u32{ 'g', 'c', 'd' };
pub fn math_Gcd(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "gcd() function only takes 2 argument",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "gcd(a,b) requires both argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(getGcd(
        values[0].asNumber(),
        values[1].asNumber(),
    ));
}

pub const NameFuncLcm = &[_]u32{ 'l', 'c', 'm' };
pub fn math_Lcm(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "lcm() function only takes 2 argument",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "lcm(a,b) requires both argument to be numbers",
        ).?;
    }

    const a = values[0].asNumber();
    const b = values[1].asNumber();

    return PValue.makeNumber((a * b) / getGcd(a, b));
}

pub const NameFuncSine = &[_]u32{ 's', 'i', 'n' };
pub fn math_Sine(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "sin(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "lcm(a) requires argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(std.math.sin(values[0].asNumber()));
}

pub const NameFuncCosine = &[_]u32{ 'c', 'o', 's' };
pub fn math_Cosine(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "cos(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "cos(a) requires argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(std.math.cos(values[0].asNumber()));
}

pub const NameFuncTangent = &[_]u32{ 't', 'a', 'n' };
pub fn math_Tangent(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "tan(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "tan(a) requires argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(std.math.tan(values[0].asNumber()));
}

pub const NameFuncDegree = &[_]u32{ 'd', 'e', 'g' };
pub fn math_Degree(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "deg(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "deg(a) requires argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(values[0].asNumber() * (180 / CONST_PI));
}

pub const NameFuncRadians = &[_]u32{ 'r', 'a', 'd' };
pub fn math_Radians(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "rad(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "rad(a) requires argument to be numbers",
        ).?;
    }

    return PValue.makeNumber(values[0].asNumber() * (CONST_PI / 180));
}

pub const NameFuncNumber = &[_]u32{ 'n', 'u', 'm' };
pub fn math_Number(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "num(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isString()) {
        return PValue.makeError(
            vm.gc,
            "num(a) requires argument to be string",
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

pub const NameFuncAbs = &[_]u32{ 'a', 'b', 's' };
pub fn math_Abs(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "num(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "num(a) requires argument to be number",
        ).?;
    }

    return PValue.makeNumber(@abs(values[0].asNumber()));
}

pub const NameFuncRound = &[_]u32{ 'r', 'o', 'u', 'n', 'd' };
pub fn math_Round(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "round(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "round(a) requires argument to be number",
        ).?;
    }

    return PValue.makeNumber(std.math.round(values[0].asNumber()));
}

pub const NameFuncFloor = &[_]u32{ 'f', 'l', 'o', 'o', 'r' };
pub fn math_Floor(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "floor(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "floor(a) requires argument to be number",
        ).?;
    }

    return PValue.makeNumber(std.math.floor(values[0].asNumber()));
}

pub const NameFuncCeil = &[_]u32{ 'c', 'e', 'i', 'l' };
pub fn math_Ceil(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "ceil(a) function only takes 1 argument",
        ).?;
    }

    if (!values[0].isNumber()) {
        return PValue.makeError(
            vm.gc,
            "ceil(a) requires argument to be number",
        ).?;
    }

    return PValue.makeNumber(std.math.ceil(values[0].asNumber()));
}
