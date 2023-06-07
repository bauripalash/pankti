//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

pub const ObjType = enum(u8) {
    String,
    Function,
    Native,
    Closure,
    Upval,
    Mod,
    Array,
    Err,
    Hmap,
};

pub const PObj = struct {
    otype : ObjType,
    isMarked : bool,
};
