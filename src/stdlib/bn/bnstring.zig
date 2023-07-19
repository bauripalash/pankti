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
const enstring = @import("../string.zig");

//স্ট্রিং
pub const Name = &[_]u32{ 0x09b8, 0x09cd, 0x099f, 0x09cd, 0x09b0, 0x09bf, 0x0982 };

pub const BnNameFuncString = &[_]u32{ 0x09b8, 0x09cd, 0x099f, 0x09cd, 0x09b0, 0x09bf, 0x0982 };
pub fn bnstr_String(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeError(vm.gc, "স্ট্রিং(ক) মাত্র একটি চলরাশি নেয়।").?;
    }

    return enstring.str_String(vm, argc, values);
}
pub const BnNameFuncSplit = &[_]u32{ 0x09ad, 0x09be, 0x0997 };
pub fn bnstr_Split(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeError(vm.gc, "ভাগ(ক, খ) কাজটি মাত্র দুটি চলরাশি নেয়।").?;
    }

    if (!values[0].isString() or !values[0].isString()) {
        return PValue.makeError(vm.gc, "ভাগ(ক , খ) এর জন্য প্রদত্ত রাশিগুলি স্ট্রিং/নাম হতে হবে। ").?;
    }

    return enstring.str_Split(vm, argc, values);
}
