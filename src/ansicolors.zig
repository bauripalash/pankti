//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0
const std = @import("std");
const writer = @import("writer.zig");
pub const ANSI_COLOR_BLACK = "\x1b[30m";
pub const ANSI_COLOR_RED = "\x1b[31m";
pub const ANSI_COLOR_GREEN = "\x1b[32m";
pub const ANSI_COLOR_YELLOW = "\x1b[33m";
pub const ANSI_COLOR_BLUE = "\x1b[34m";
pub const ANSI_COLOR_PURPLE = "\x1b[35m";
pub const ANSI_COLOR_CYAN = "\x1b[36m";
pub const ANSI_COLOR_WHITE = "\x1b[37m";
pub const ANSI_COLOR_RESET = "\x1b[0m";

pub fn TermColor(color: u8, w: std.io.AnyWriter) void {
    switch (color) {
        'B' => {
            w.print("{s}", .{ANSI_COLOR_BLACK}) catch return;
        },
        'r' => {
            w.print("{s}", .{ANSI_COLOR_RED}) catch return;
        },
        'g' => {
            w.print("{s}", .{ANSI_COLOR_GREEN}) catch return;
        },
        'y' => {
            w.print("{s}", .{ANSI_COLOR_YELLOW}) catch return;
        },
        'b' => {
            w.print("{s}", .{ANSI_COLOR_BLUE}) catch return;
        },
        'p' => {
            w.print("{s}", .{ANSI_COLOR_PURPLE}) catch return;
        },
        'c' => {
            w.print("{s}", .{ANSI_COLOR_CYAN}) catch return;
        },
        'w' => {
            w.print("{s}", .{ANSI_COLOR_WHITE}) catch return;
        },
        else => {
            return;
        },
    }
}

pub fn ResetColor(w: std.io.AnyWriter) void {
    w.print("{s}", .{ANSI_COLOR_RESET}) catch return;
}
