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
const Gc = @import("../gc.zig").Gc;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const msl = stdlib.msl;
const builtin = @import("builtin");

const CONST_PI: f64 = 3.14159265358979323846;
const CONST_E: f64 = 2.71828182845904523536;

pub const Name = &[_]u32{ 'm', 'a', 't', 'h' };

pub const NameFuncPi = &[_]u32{ 'p', 'i' };
pub fn math_Pi(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;

    if (argc != 0) {
        return PValue.makeError(
            gc,
            "pi() function takes no argument",
        ).?;
    }

    return PValue.makeNumber(CONST_PI);
}

pub const NameFuncE = &[_]u32{'e'};
pub fn math_E(gc: *Gc, argc: u8, values: []PValue) PValue {
    _ = values;

    if (argc != 0) {
        return PValue.makeError(
            gc,
            "e() function takes no argument",
        ).?;
    }

    return PValue.makeNumber(CONST_E);
}

pub const NameFuncSqrt = &[_]u32{ 's', 'q', 'r', 't' };
pub fn math_Sqrt(gc: *Gc, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            gc,
            "sqrt() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            gc,
            "sqrt() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(@sqrt(rawValue.asNumber()));
}

pub const NameFuncLog10 = &[_]u32{ 'l', 'o', 'g', '1', '0' };
pub fn math_Log10(gc: *Gc, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            gc,
            "log10() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            gc,
            "log10() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(@log10(rawValue.asNumber()));
}

pub const NameFuncLog = &[_]u32{ 'l', 'o', 'g' };
pub fn math_Log(gc: *Gc, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            gc,
            "log() function only takes single argument",
        ).?;
    }

    const rawValue = values[0];
    if (!rawValue.isNumber()) {
        return PValue.makeError(
            gc,
            "log() function only works on numbers",
        ).?;
    }

    return PValue.makeNumber(@log(rawValue.asNumber()));
}

pub const NameFuncLogX = &[_]u32{ 'l', 'o', 'g', 'x' };
pub fn math_LogX(gc: *Gc, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            gc,
            "logx() function only takes 2 argument",
        ).?;
    }

    if (!values[0].isNumber() or !values[1].isNumber()) {
        return PValue.makeError(
            gc,
            "logx(a,b) required both argument to be numbers",
        ).?;
    }
    const rawBase = values[0].asNumber();
    const rawNum = values[1].asNumber();

    return PValue.makeNumber(@log(rawNum) / @log(rawBase));
}
