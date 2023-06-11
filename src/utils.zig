//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

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

test "test utils->u8tou32->english" {
    const al = std.testing.allocator;
    const text = "The quick brown fox jumps over the lazy dog";

    const textU32 = try u8tou32(text, al);

    const textU32X = [_]u32{ 84, 104, 101, 32, 113, 117, 105, 99, 107, 32, 98, 114, 111, 119, 110, 32, 102, 111, 120, 32, 106, 117, 109, 112, 115, 32, 111, 118, 101, 114, 32, 116, 104, 101, 32, 108, 97, 122, 121, 32, 100, 111, 103 };

    try std.testing.expectEqual(text.len, textU32.len);
    try std.testing.expectEqual(textU32.len, textU32X.len);

    for (textU32, 0..) |value, i| {
        try std.testing.expectEqual(value, textU32X[i]);
    }

    al.free(textU32);
}

test "test utils->u8tou32->bengali" {
    const al = std.testing.allocator;
    const text =
        \\জীর্ণ পৃথিবীতে ব্যর্থ, মৃত আর ধ্বংসস্তূপ-পিঠে
        \\ চলে যেতে হবে আমাদের।
        \\ চলে যাব- তবু আজ যতক্ষণ দেহে আছে প্রাণ
        \\ প্রাণপণে পৃথিবীর সরাব জঞ্জাল,
        \\ এ বিশ্বকে এ শিশুর বাসযোগ্য ক’রে যাব আমি
        \\নবজাতকের কাছে এ আমার দৃঢ় অঙ্গীকার।
    ;
    const textU32 = try u8tou32(text, al);

    const textU32X = [_]u32{ 2460, 2496, 2480, 2509, 2467, 32, 2474, 2499, 2469, 2495, 2476, 2496, 2468, 2503, 32, 2476, 2509, 2479, 2480, 2509, 2469, 44, 32, 2478, 2499, 2468, 32, 2438, 2480, 32, 2471, 2509, 2476, 2434, 2488, 2488, 2509, 2468, 2498, 2474, 45, 2474, 2495, 2464, 2503, 10, 32, 2458, 2482, 2503, 32, 2479, 2503, 2468, 2503, 32, 2489, 2476, 2503, 32, 2438, 2478, 2494, 2470, 2503, 2480, 2404, 10, 32, 2458, 2482, 2503, 32, 2479, 2494, 2476, 45, 32, 2468, 2476, 2497, 32, 2438, 2460, 32, 2479, 2468, 2453, 2509, 2487, 2467, 32, 2470, 2503, 2489, 2503, 32, 2438, 2459, 2503, 32, 2474, 2509, 2480, 2494, 2467, 10, 32, 2474, 2509, 2480, 2494, 2467, 2474, 2467, 2503, 32, 2474, 2499, 2469, 2495, 2476, 2496, 2480, 32, 2488, 2480, 2494, 2476, 32, 2460, 2462, 2509, 2460, 2494, 2482, 44, 10, 32, 2447, 32, 2476, 2495, 2486, 2509, 2476, 2453, 2503, 32, 2447, 32, 2486, 2495, 2486, 2497, 2480, 32, 2476, 2494, 2488, 2479, 2507, 2455, 2509, 2479, 32, 2453, 128, 38950, 2087, 30752, 27616, 28576, 27424, 2438, 2478, 2495, 10, 2472, 2476, 2460, 2494, 2468, 2453, 2503, 2480, 32, 2453, 2494, 2459, 2503, 32, 2447, 32, 2438, 2478, 2494, 2480, 32, 2470, 2499, 2466, 2492, 32, 2437, 2457, 2509, 2455, 2496, 2453, 2494, 2480, 2404 };

    try std.testing.expectEqual(textU32X.len, textU32.len);

    for (textU32, 0..) |value, i| {
        try std.testing.expectEqual(value, textU32X[i]);
    }

    al.free(textU32);
}

test "test utils->u32tou8->english" {
    const al = std.testing.allocator;
    const textU32 = [_]u32{ 84, 104, 101, 32, 113, 117, 105, 99, 107, 32, 98, 114, 111, 119, 110, 32, 102, 111, 120, 32, 106, 117, 109, 112, 115, 32, 111, 118, 101, 114, 32, 116, 104, 101, 32, 108, 97, 122, 121, 32, 100, 111, 103 };

    const textU8 = try u32tou8(&textU32, al);

    const text = "The quick brown fox jumps over the lazy dog";

    try std.testing.expectEqual(textU8.len, textU8.len);

    for (textU8, 0..) |value, i| {
        try std.testing.expectEqual(value, text[i]);
    }

    al.free(textU8);
}


