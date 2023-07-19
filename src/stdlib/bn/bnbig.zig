//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const value = @import("../../value.zig");
const Vm = @import("../../vm.zig").Vm;
const PValue = value.PValue;
const enbig = @import("../big.zig");

//বড়
pub const Name = &[_]u32{ 0x09ac, 0x09dc };

pub const BnNamefuncSub = &[_]u32{ 0x09ac, 0x09bf, 0x09df, 0x09cb, 0x0997 };
pub fn bnbig_Sub(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enbig.big_Sub(vm, argc, values);
}

pub const BnNamefuncAdd = &[_]u32{ 0x09af, 0x09cb, 0x0997 };
pub fn bnbig_Add(vm: *Vm, argc: u8, values: []PValue) PValue {
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

    return enbig.big_Add(vm, argc, values);
}

pub const BnNameFuncNew = &[_]u32{ 0x09a8, 0x09a4, 0x09c1, 0x09a8 };
pub fn bnbig_New(vm: *Vm, argc: u8, values: []PValue) PValue {
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
    return enbig.big_New(vm, argc, values);
}
