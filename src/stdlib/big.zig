//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const big = std.math.big;
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");
const stdlib = @import("stdlib.zig");
const PObj = @import("../object.zig").PObj;

//বড়
pub const Name: []const u8 = "বড়";

pub const NamefuncSub: []const u8 = "বিয়োগ";
pub fn big_Sub(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "বিয়োগ(ক , খ) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "বিয়োগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "বিয়োগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    x.ival.sub(&aInt.ival, &bInt.ival) catch return PValue.makeNil();

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncAdd: []const u8 = "যোগ";
pub fn big_Add(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "যোগ(ক , খ) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "যোগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "যোগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }
    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    x.ival.add(&aInt.ival, &bInt.ival) catch {
        return PValue.makeNil();
    };

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncDiv: []const u8 = "ভাগ";
pub fn big_Div(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "ভাগ(ক , খ) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "ভাগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "ভাগ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }
    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    var temp = big.int.Managed.init(vm.gc.hal()) catch {
        return PValue.makeNil();
    };

    defer temp.deinit();

    x.ival.divFloor(&temp, &aInt.ival, &bInt.ival) catch {
        return PValue.makeNil();
    };

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncRem: []const u8 = "ভাগশেষ";
pub fn big_Rem(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "ভাগশেষ(ক , খ) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "ভাগশেষ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "ভাগশেষ(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }
    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    var temp = big.int.Managed.init(vm.gc.hal()) catch {
        return PValue.makeNil();
    };

    defer temp.deinit();

    temp.ival.divFloor(&x.ival, &aInt.ival, &bInt.ival) catch {
        return PValue.makeNil();
    };

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncMul: []const u8 = "গুন";
pub fn big_Mul(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(
            vm.gc,
            "গুন(ক , খ) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];
    const b = values[1];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "গুন(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    if (!b.isObj() or !b.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "গুন(ক , খ) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }
    const aInt = a.asObj().asBigInt();
    const bInt = b.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    x.ival.mul(&aInt.ival, &bInt.ival) catch {
        return PValue.makeNil();
    };

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NamefuncSqrt: []const u8 = "বর্গমূল";
pub fn big_Sqrt(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "বর্গমূল(ক) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে।",
        ).?;
    }

    const a = values[0];

    if (!a.isObj() or !a.asObj().isBigInt()) {
        return PValue.makeError(
            vm.gc,
            "বর্গমূল(ক) কাজটি মাত্র বড় সংখ্যা গ্রহণ করে",
        ).?;
    }

    const aInt = a.asObj().asBigInt();

    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    if (!x.initInt(vm.gc)) return PValue.makeNil();

    x.ival.sqrt(&aInt.ival) catch {
        return PValue.makeNil();
    };

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    return vm.stack.pop() catch return PValue.makeNil();
}

pub const NameFuncNew: []const u8 = "নতুন";
pub fn big_New(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(
            vm.gc,
            "নতুন(ক) কাজটি শুধুমাত্র একটি চলরাশি গ্রহণ করে",
        ).?;
    }

    const item = values[0];

    if (!item.isString() and !item.isNumber()) {
        return PValue.makeError(
            vm.gc,
            "নতুন(ক) কাজটি শুধুমাত্র সংখ্যা কিংবা স্ট্রিং গ্রহণ করে",
        ).?;
    }
    const x: *PObj.OBigInt = vm.gc.newObj(.Ot_BigInt, PObj.OBigInt) catch {
        return PValue.makeNil();
    };

    _ = x.initInt(vm.gc);

    vm.stack.push(x.parent().asValue()) catch return PValue.makeNil();

    if (item.isString()) {
        const rawString = item.asObj().asString();

        const u8string = rawString.chars;

        x.ival.setString(10, u8string) catch {
            return PValue.makeComptimeError(
                vm.gc,
                "নতুন(...) কাজটিতে অবৈধ স্ট্রিং \"{s}\" দেওয়া হয়েছে!",
                .{u8string},
            ).?;
        };
    } else if (item.isNumber()) {
        const number = item.asNumber();
        const f_temp: i64 = std.math.lossyCast(isize, @trunc(number));
        x.ival.set(f_temp) catch return PValue.makeNil();
    }

    return vm.stack.pop() catch return PValue.makeNil();
}
