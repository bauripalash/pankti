//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("value.zig");
const PValue = value.PValue;

pub fn nClock(argc: u8, values: []PValue) PValue {
    _ = values;
    _ = argc;
    const s = std.time.milliTimestamp();
    return PValue.makeNumber(@floatFromInt(s));
}

pub fn nShow(argc: u8, values: []PValue) PValue {
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        values[i].printVal();
        std.debug.print("\n", .{}); //Until complete
    }
    return PValue.makeNil();
}

pub fn nBnShow(argc: u8, values: []PValue) PValue { //WILL BE CHANGED?
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        values[i].printVal();
        std.debug.print("\n", .{}); //Until complete
    }
    return PValue.makeNil();
}
