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

    var fileToRun: ?[]u8 = null;

    var args = try std.process.argsAlloc(ga);

    if (args.len == 2) {
        fileToRun = try ga.alloc(u8, args[1].len);
        @memcpy(fileToRun.?, args[1]);
    }
    std.process.argsFree(ga, args);
    defer {
        _ = gpa.deinit();
    }
    //print("{s}" , .{fileToRun.?});
    if (fileToRun) |f| {
        const text = try openfile(f, ga);
        const u = try utils.u8tou32(text, ga);
        var l = lexer.Lexer.new(u);
        l.debug();
        ga.free(u);
        ga.free(f);
        ga.free(text);
    }
    //const text = try openfile("a.txt", ga);
    //const text = "show(100+22.0 == 5.9-2.001);";
    //print("->{any}\n", .{@TypeOf(text)});

}
