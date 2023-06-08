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
/// You must free the result
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

/// Convert UTF-32 encoded string to UTF-8 encoded string 
/// You must free the result
pub fn u32tou8(input: []const u32, al: std.mem.Allocator) ![]u8 {
    const len32 = input.len;
    var u8str = try al.alloc(u8, len32 * 4);

    var i: usize = 0;
    for (input) |value| {
        //std.debug.print("\n->{any}|{}\n", .{value , value});
        if (value <= 0x7F) {
            u8str[i] = @intCast(u8, value); //1
            i += 1;
        } else if (value <= 0x7FF) {
            u8str[i] = (0xC0 | @intCast(u8, (value >> 6) & 0x1F));
            u8str[i + 1] = (0x80 | @intCast(u8, value & 0x3F));
            i += 2;
        } else if (value <= 0xFFFF) {
            u8str[i] = (0xE0 | @intCast(u8, (value >> 12) & 0x0F));
            u8str[i + 1] = (0x80 | @intCast(u8, (value >> 6) & 0x3F));
            u8str[i + 2] = (0x80 | @intCast(u8, value & 0x3F));
            i += 3;
        } else {
            u8str[i] = (0xF0 | @intCast(u8, (value >> 18) & 0x07));
            u8str[i + 1] = (0x80 | @intCast(u8, (value >> 12) & 0x3F));
            u8str[i + 2] = (0x80 | @intCast(u8, (value >> 6) & 0x3F));
            u8str[i + 3] = (0x80 | @intCast(u8, value & 0x3F));
            i += 4;
        }
    }

    u8str = al.realloc(u8str, i) catch u8str;

    return u8str;
}

/// Print a UTF-32 encoded string to stdout
pub fn printu32(input: []const u32) void {
    for (input) |value| {
        std.debug.print("{u}", .{@truncate(u21, value)});
    }
}

/// Check if `c` is valid english number
pub fn isEnNum(c: u32) bool {
    return c <= '9' and c >= '0';
}

/// Check if `c` is valid english letter or underscore `_`
pub fn isValidEn(c: u32) bool {
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

/// Check if `a` is same as `b`
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
