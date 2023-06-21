//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

pub const ANSI_COLOR_BLACK  = "\x1b[30m";
pub const ANSI_COLOR_RED    = "\x1b[31m";
pub const ANSI_COLOR_GREEN  = "\x1b[32m";
pub const ANSI_COLOR_YELLOW = "\x1b[33m";
pub const ANSI_COLOR_BLUE   = "\x1b[34m";
pub const ANSI_COLOR_PURPLE = "\x1b[35m";
pub const ANSI_COLOR_CYAN   = "\x1b[36m";
pub const ANSI_COLOR_WHITE  = "\x1b[37m";
pub const ANSI_COLOR_RESET  = "\x1b[0m";

pub fn TermColor(color : u8) void{
    switch (color) {
        'B' => {std.debug.print("{s}" , .{ANSI_COLOR_BLACK});},
        'r' => {std.debug.print("{s}" , .{ANSI_COLOR_RED});},
        'g' => {std.debug.print("{s}" , .{ANSI_COLOR_GREEN});},
        'y' => {std.debug.print("{s}" , .{ANSI_COLOR_YELLOW});},
        'b' => {std.debug.print("{s}" , .{ANSI_COLOR_BLUE});},
        'p' => {std.debug.print("{s}" , .{ANSI_COLOR_PURPLE});},
        'c' => {std.debug.print("{s}" , .{ANSI_COLOR_CYAN});},
        'w' => {std.debug.print("{s}" , .{ANSI_COLOR_WHITE});},
        else => { return; }
    }
}

pub fn ResetColor() void {
    std.debug.print("{s}" , .{ANSI_COLOR_RESET});
}
