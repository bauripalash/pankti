//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

pub const BN_NUM_0 = 0x09E6;
pub const BN_NUM_1 = 0x09E7;
pub const BN_NUM_2 = 0x09E8;
pub const BN_NUM_3 = 0x09E9;
pub const BN_NUM_4 = 0x09EA;
pub const BN_NUM_5 = 0x09EB;
pub const BN_NUM_6 = 0x09EC;
pub const BN_NUM_7 = 0x09ED;
pub const BN_NUM_8 = 0x09EE;
pub const BN_NUM_9 = 0x09EF;

pub const BN_RANGE_START = 0x0980;
pub const BN_RANGE_END = 0x09FE;

/// Cheks if `c`, a UTF-32 encoded `char` is a valid bengali number
pub inline fn isBnNumber(c: u32) bool {
    return c >= BN_NUM_0 and c <= BN_NUM_9;
}

/// Check if `c` is in bengali unicode range;
/// Doesn't check for invalid or reserved chars
pub inline fn isBnChar(c: u32) bool {
    return c >= BN_RANGE_START and c <= BN_RANGE_END;
}

pub inline fn bnToEnNum(c: u21) u21 {
    return switch (c) {
        BN_NUM_0 => '0',
        BN_NUM_1 => '1',
        BN_NUM_2 => '2',
        BN_NUM_3 => '3',
        BN_NUM_4 => '4',
        BN_NUM_5 => '5',
        BN_NUM_6 => '6',
        BN_NUM_7 => '7',
        BN_NUM_8 => '8',
        BN_NUM_9 => '9',
        else => c,
    };
}
