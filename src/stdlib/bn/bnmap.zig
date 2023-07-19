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
const enmap = @import("../map.zig");

//ম্যাপ
pub const Name = &[_]u32{ 0x09ae, 0x09cd, 0x09af, 0x09be, 0x09aa };

pub const BnNameFuncExists = &[_]u32{ 0x09ac, 0x09b0, 0x09cd, 0x09a4, 0x09ae, 0x09be, 0x09a8 };
pub fn bnmap_Exists(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(vm.gc, "বর্তমান(ম্যাপ, সূচক) কাজটি মাত্র দুটি চলরাশি গ্রহণ করে").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "বর্তমান(ম্যাপ, সূচক) কাজটির প্রথম চলরাশিটি একটি ম্যাপ হতে হবে").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "বর্তমান(ম্যাপ, সূচক) কাজটির প্রথম চলরাশিটি একটি ম্যাপ হতে হবে").?;
    }

    return enmap.map_Exists(vm, argc, values);
}
pub const BnNameFuncKeys = &[_]u32{ 0x09b8, 0x09c2, 0x099a, 0x0995 };
pub fn bnmap_Keys(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "সূচক(ম্যাপ) কাজটি মাত্র একটি চলরাশি গ্রহণ করে।").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "সূচক(ম্যাপ) কাজটিতে প্রদত্ত চলরাশি ম্যাপ হতে হবে").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "সূচক(ম্যাপ) কাজটিতে প্রদত্ত চলরাশি ম্যাপ হতে হবে").?;
    }

    return enmap.map_Keys(vm, argc, values);
}

pub const BnNameFuncValues = &[_]u32{ 0x09ae, 0x09be, 0x09a8 };

pub fn bnmap_Values(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "মান(ম্যাপ) কাজটি মাত্র একটি চলরাশি গ্রহণ করে।").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "মান(ম্যাপ) কাজটিতে প্রদত্ত চলরাশি ম্যাপ হতে হবে").?;
    }

    if (!values[0].asObj().isHmap()) {
        return PValue.makeError(vm.gc, "মান(ম্যাপ) কাজটিতে প্রদত্ত চলরাশি ম্যাপ হতে হবে").?;
    }
    return enmap.map_Values(vm, argc, values);
}
