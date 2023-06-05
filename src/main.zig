//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

const std = @import("std");
const openfile = @import("openfile.zig").openfile;
const lexer = @import("lexer.zig");
const print = std.debug.print;
const utils = @import("utils.zig");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const ga = gpa.allocator();
    defer {
        _ = gpa.deinit();
    }
    //const text = try openfile("a.txt", ga);
    const text = "show(100+220 == 5-2);";
    const u = try utils.u8tou32(text, ga);
    //print("->{any}\n", .{@TypeOf(text)});

    var l = lexer.Lexer.new(u);

    l.debug();

    ga.free(u);
}
