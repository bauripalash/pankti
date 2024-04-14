//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

pub const K_EN_LET = [_]u32{ 'l', 'e', 't' };
pub const K_EN_SHOW = [_]u32{ 's', 'h', 'o', 'w' };
pub const K_EN_AND = [_]u32{ 'a', 'n', 'd' };
pub const K_EN_OR = [_]u32{ 'o', 'r' };
pub const K_EN_END = [_]u32{ 'e', 'n', 'd' };
pub const K_EN_IF = [_]u32{ 'i', 'f' };
pub const K_EN_THEN = [_]u32{ 't', 'h', 'e', 'n' };
pub const K_EN_ELSE = [_]u32{ 'e', 'l', 's', 'e' };
pub const K_EN_WHILE = [_]u32{ 'w', 'h', 'i', 'l', 'e' };
pub const K_EN_NIL = [_]u32{ 'n', 'i', 'l' };
pub const K_EN_TRUE = [_]u32{ 't', 'r', 'u', 'e' };
pub const K_EN_FALSE = [_]u32{ 'f', 'a', 'l', 's', 'e' };
pub const K_EN_RETURN = [_]u32{ 'r', 'e', 't', 'u', 'r', 'n' };
pub const K_EN_FUNC = [_]u32{ 'f', 'u', 'n', 'c' };
pub const K_EN_IMPORT = [_]u32{ 'i', 'm', 'p', 'o', 'r', 't' };
pub const K_EN_PANIC = [_]u32{ 'p', 'a', 'n', 'i', 'c' };
pub const K_EN_DO = [_]u32{ 'd', 'o' };
pub const K_EN_BREAK = [_]u32{ 'b', 'r', 'e', 'a', 'k' };

pub const K_BN_LET = [_]u32{ 0x09a7, 0x09b0, 0x09bf };
pub const K_BN_SHOW = [_]u32{ 0x09a6, 0x09c7, 0x0996, 0x09be, 0x0993 };
pub const K_BN_AND = [_]u32{ 0x098f, 0x09ac, 0x0982 };
pub const K_BN_OR = [_]u32{ 0x09ac, 0x09be };
pub const K_BN_END = [_]u32{ 0x09b6, 0x09c7, 0x09b7 };
pub const K_BN_IF = [_]u32{ 0x09af, 0x09a6, 0x09bf };
pub const K_BN_THEN = [_]u32{ 0x09a4, 0x09be, 0x09b9, 0x09b2, 0x09c7 };
pub const K_BN_ELSE = [_]u32{ 0x09a8, 0x09be, 0x09b9, 0x09b2, 0x09c7 };
pub const K_BN_WHILE = [_]u32{
    0x09af,
    0x09a4,
    0x0995,
    0x09cd,
    0x09b7,
    0x09a3,
};
pub const K_BN_NIL = [_]u32{ 0x09a8, 0x09bf, 0x09b2 };
pub const K_BN_TRUE = [_]u32{ 0x09b8, 0x09a4, 0x09cd, 0x09af, 0x09bf };
pub const K_BN_FALSE = [_]u32{
    0x09ae,
    0x09bf,
    0x09a5,
    0x09cd,
    0x09af,
    0x09be,
};
pub const K_BN_RETURN = [_]u32{ 0x09ab, 0x09c7, 0x09b0, 0x09be, 0x0993 };
pub const K_BN_FUNC = [_]u32{ 0x0995, 0x09be, 0x099c };
pub const K_BN_IMPORT = [_]u32{ 0x0986, 0x09a8, 0x09df, 0x09a8 };
pub const K_BN_PANIC = [_]u32{
    0x0997,
    0x09cb,
    0x09b2,
    0x09ae,
    0x09be,
    0x09b2,
};
pub const K_BN_DO = [_]u32{ 0x0995, 0x09b0, 0x09cb };
pub const K_BN_BREAK = [_]u32{ 0x09ad, 0x09be, 0x0999, 0x09cb };

pub const K_PN_LET = [_]u32{ 'd', 'h', 'o', 'r', 'i' };
pub const K_PN_SHOW = [_]u32{ 'd', 'e', 'k', 'h', 'a', 'u' };
pub const K_PN_AND = [_]u32{ 'e', 'b', 'o', 'n', 'g' };
pub const K_PN_OR = [_]u32{ 'b', 'a' };
pub const K_PN_END = [_]u32{ 's', 'e', 's', 'h' };
pub const K_PN_IF = [_]u32{ 'j', 'o', 'd', 'i' };
pub const K_PN_THEN = [_]u32{ 't', 'a', 'h', 'o', 'l', 'e' };
pub const K_PN_ELSE = [_]u32{ 'n', 'a', 'h', 'o', 'l', 'e' };
pub const K_PN_WHILE = [_]u32{ 'j', 'o', 't', 'o', 'k', 'h', 'o', 'n' };
pub const K_PN_NIL = [_]u32{ 'n', 'i', 'l' };
pub const K_PN_TRUE = [_]u32{ 's', 'o', 't', 't', 'i' };
pub const K_PN_FALSE = [_]u32{ 'm', 'i', 't', 't', 'h', 'a' };
pub const K_PN_RETURN = [_]u32{ 'f', 'e', 'r', 'a', 'u' };
pub const K_PN_FUNC = [_]u32{ 'k', 'a', 'j' };
pub const K_PN_IMPORT = [_]u32{ 'a', 'n', 'o', 'y', 'o', 'n' };
pub const K_PN_PANIC = [_]u32{ 'p', 'a', 'n', 'i', 'c' };
pub const K_PN_DO = [_]u32{ 'k', 'o', 'r', 'o' };
pub const K_PN_BREAK = [_]u32{ 'b', 'h', 'a', 'n', 'g', 'o' };
