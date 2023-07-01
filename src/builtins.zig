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
const Gc = @import("gc.zig").Gc;
const PValue = value.PValue;

pub fn nClock(gc : *Gc , argc: u8, values: []PValue) PValue {
    _ = gc;
    _ = values;
    _ = argc;
    const s = std.time.milliTimestamp();
    return PValue.makeNumber(@floatFromInt(s));
}

pub fn nShow(gc : *Gc , argc: u8, values: []PValue) PValue {
    _ = gc;
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        values[i].printVal();
        std.debug.print("\n", .{}); //Until complete
    }
    return PValue.makeNil();
}

pub fn nBnShow(gc : *Gc , argc: u8, values: []PValue) PValue {
    _ = gc; //WILL BE CHANGED?
    var i: usize = 0;
    while (i < argc) : (i += 1) {
        values[i].printVal();
        std.debug.print("\n", .{}); //Until complete
    }
    return PValue.makeNil();
}
