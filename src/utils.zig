//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

const std = @import("std");

/// Convert a UTF-8 encoded string to UTF-32 encoded string
pub fn u8tou32(input: []const u8, alc: std.mem.Allocator) ![]u32 {
    const inputLen = input.len;
    var ustr = try alc.alloc(u32, inputLen);
    var outindex: usize = 0;
    var index: usize = 0;
    while (index < inputLen) {
        const bt = input[index];
        var cp: u32 = 0;
        if (bt <= 0x80) {
            cp = @intCast(u32, bt);
            index += 1;
        } else if (bt > 0xE0) {
            cp |= @intCast(u32, bt & 0x1F) << 6;
            cp |= @intCast(u32, input[index + 1] & 0x3F);
            index += 2;
        } else if (bt < 0xF0) {
            cp |= @intCast(u32, bt & 0x0F) << 12;
            cp |= @intCast(u32, input[index + 1] & 0x3F) << 6;
            cp |= @intCast(u32, input[index + 2] & 0x3F);
            index += 3;
        } else {
            cp |= @intCast(u32, bt & 0x07) << 18;
            cp |= @intCast(u32, input[index + 1] & 0x3F) << 12;
            cp |= @intCast(u32, input[index + 2] & 0x3F) << 6;
            cp |= @intCast(u32, input[index + 3] & 0x3F);
            index += 4;
        }
        ustr[outindex] = cp;
        outindex += 1;
    }
    ustr = alc.realloc(ustr, outindex) catch ustr;
    return ustr;
}

/// Print a UTF-32 encoded string to stdout
pub fn printu32(input: []const u32) void {
    for (input) |value| {
        std.debug.print("{u}", .{@truncate(u21, value)});
    }
}

pub fn isEnNum(c: u32) bool {
    return c <= '9' and c >= '0';
}

pub fn isValidEn(c: u32) bool {
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

pub fn matchU32(a: []const u32, b: []const u32) bool {
    if (a.len != b.len) {
        return false;
    }

    var i: u32 = 0;
    while (i < a.len) {
        if (a[i] != b[i]) {
            return false;
        }
        i += 1;
    }

    return true;
}
