//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const print = std.debug.print;

pub fn openfile(path: []const u8, aloc: std.mem.Allocator) ![]u8 {
    var f = try std.fs.cwd().openFile(path, .{});
    defer f.close();

    const read_buf = try f.readToEndAlloc(aloc, 2048);
    return read_buf;
}
