//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const builtin = @import("builtin");
const writer = @import("writer.zig");
const Gc = @import("gc.zig").Gc;
const bn = @import("bengali/bn.zig");

pub extern fn getTimestamp() usize;

pub fn matchU8(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}

pub fn matchIdent(
    i: []const u8,
    a: []const u8,
    b: []const u8,
    c: []const u8,
) bool {
    if (std.mem.eql(u8, i, a) or std.mem.eql(u8, i, b) or std.mem.eql(
        u8,
        i,
        c,
    )) {
        return true;
    }
    return false;
}

pub fn isInt(v: f64) bool {
    return @ceil(v) == v;
}

pub fn asInt(v: f64) i64 {
    return @intFromFloat(@ceil(v));
}

pub fn asUint(v: f64) u64 {
    return @abs(asInt(v));
}

pub fn strBnToEnNum(al: std.mem.Allocator, input: []const u32, len: usize) ![]u8 {
    var i: usize = 0;
    const result = try al.alloc(u32, len);
    while (i < len) : (i += 1) {
        result[i] = bn.bnToEnNum(input[i]);
    }

    return result;
}

pub const UTFError = error{
    InvalidUTF8ByteLength,
    InvalidUTF32Byte,
};

pub fn getUtf32LenFor8(input: []const u8, len: usize) UTFError!usize {
    var result: usize = 0;
    var index: usize = 0;

    while (index < len) {
        const b = input[index];
        const blen = std.unicode.utf8ByteSequenceLength(b) catch {
            return UTFError.InvalidUTF8ByteLength;
        };

        index += blen;
        result += 1;
    }

    return result;
}

pub fn getUTF8LenFor32(input: []const u32) UTFError!usize {
    var result: usize = 0;

    for (input) |c32| {
        const char: u21 = @truncate(c32);
        result += std.unicode.utf8CodepointSequenceLength(char) catch {
            return UTFError.InvalidUTF32Byte;
        };
    }

    return result;
}

/// Convert a UTF-8 encoded string to UTF-32 encoded string
/// You must free the result
pub fn _u8tou32(input: []const u8, alc: std.mem.Allocator) ![]u32 {
    const inputLen = input.len;
    const outLen = try getUtf32LenFor8(input, inputLen);
    var ustr = try alc.alloc(u32, outLen);
    var outindex: usize = 0;
    var index: usize = 0;

    while (index < inputLen and outindex < outLen) {
        const bt = input[index];
        var cp: u21 = 0;
        const byteLen = std.unicode.utf8ByteSequenceLength(bt) catch {
            return UTFError.InvalidUTF8ByteLength;
        };

        if (byteLen == 1) {
            cp = @intCast(bt);
            index += 1;
        } else if (byteLen == 2) {
            cp = try std.unicode.utf8Decode2(&[2]u8{ bt, input[index + 1] });
            index += 2;
        } else if (byteLen == 3) {
            cp = try std.unicode.utf8Decode3(&[3]u8{
                bt,
                input[index + 1],
                input[index + 2],
            });
            index += 3;
        } else {
            cp = try std.unicode.utf8Decode4(&[4]u8{
                bt,
                input[index + 1],
                input[index + 2],
                input[index + 3],
            });
        }

        ustr[outindex] = @intCast(cp);
        outindex += 1;
    }

    return ustr;
}

/// Convert UTF-32 encoded string to UTF-8 encoded string
/// You must free the result
pub fn _u32tou8(
    input: []const u32,
    al: std.mem.Allocator,
) anyerror![]u8 {
    const u8Len = try getUTF8LenFor32(input);
    var u8str = try al.alloc(u8, u8Len);

    var i: usize = 0;

    for (input) |c32| {
        const char: u21 = @truncate(c32);

        const l = try std.unicode.utf8CodepointSequenceLength(char);
        //std.debug.print("\n{d}|L->{d}\n", .{ u8Len, l });
        _ = try std.unicode.utf8Encode(char, u8str[i..]);
        i += l;
    }

    //var i: usize = 0;
    //for (input) |value| {
    //    //std.debug.print("\n->{any}|{}\n", .{value , value});
    //    if (value <= 0x7F) {
    //        u8str[i] = @intCast(value); //1
    //        i += 1;
    //    } else if (value <= 0x7FF) {
    //        u8str[i] = (0xC0 | @as(u8, @intCast((value >> 6) & 0x1F)));
    //        u8str[i + 1] = (0x80 | @as(u8, @intCast(value & 0x3F)));
    //        i += 2;
    //    } else if (value <= 0xFFFF) {
    //        u8str[i] = (0xE0 | @as(u8, @intCast((value >> 12) & 0x0F)));
    //        u8str[i + 1] = (0x80 | @as(u8, @intCast((value >> 6) & 0x3F)));
    //        u8str[i + 2] = (0x80 | @as(u8, @intCast(value & 0x3F)));
    //        i += 3;
    //    } else {
    //        u8str[i] = (0xF0 | @as(u8, @intCast((value >> 18) & 0x07)));
    //        u8str[i + 1] = (0x80 | @as(u8, @intCast((value >> 12) & 0x3F)));
    //        u8str[i + 2] = (0x80 | @as(u8, @intCast((value >> 6) & 0x3F)));
    //        u8str[i + 3] = (0x80 | @as(u8, @intCast(value & 0x3F)));
    //        i += 4;
    //    }
    //}

    //u8str = al.realloc(u8str, i) catch u8str;
    //std.debug.print("-->string-->|<{d}> => <{d}>|{s}\n\n", .{ len32, i, u8str });

    return u8str;
}

pub fn hashChars(input: []const u8, _: *Gc) !u32 {
    var result: u32 = 0;

    //if (IS_WASM) {
    var hasher = std.hash.Fnv1a_32.init();
    hasher.update(input);
    result = hasher.final();

    // Do we really need XxHash?

    //} else {
    //     var hasher = std.hash.XxHash32.init(gc.timestamp);
    //    const u = try u32tou8(input, gc.hal());
    //   hasher.update(u);
    //  gc.hal().free(u);
    // result = hasher.final();
    //}
    return result;
}

/// Print a UTF-32 encoded string to stdout
pub fn printu32(input: []const u32, w: std.io.AnyWriter) void {
    for (input) |value| {
        w.print("{u}", .{@as(u21, @truncate(value))}) catch return;
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

/// Convert an u16 to u8
pub fn u16tou8(a: u16) [2]u8 {
    var result: [2]u8 = undefined;
    result[0] = @intCast(a >> 8);
    result[1] = @intCast(a & 0xff);
    return result;
}

/// Convert two u8 to u16
pub fn u8tou16(a: []const u8) u16 {
    if (a.len > 2) {
        return 0;
    }
    const result: u16 = (@as(u16, @intCast(a[0])) << 8) | @as(u16, @intCast(a[1]));
    return result;
}

pub const IS_WASM = (builtin.target.isWasm() and builtin.target.os.tag == .freestanding);

pub const IS_WIN: bool = builtin.target.os.tag == .windows;
pub const IS_MAC: bool = builtin.target.isDarwin();
pub const IS_LINUX: bool = builtin.target.os.tag == .linux;

test "test utils->u8tou32->english" {
    const al = std.testing.allocator;
    const text = "The quick brown fox jumps over the lazy dog";

    const textU32 = try _u8tou32(text, al);

    const textU32X = [_]u32{
        84,
        104,
        101,
        32,
        113,
        117,
        105,
        99,
        107,
        32,
        98,
        114,
        111,
        119,
        110,
        32,
        102,
        111,
        120,
        32,
        106,
        117,
        109,
        112,
        115,
        32,
        111,
        118,
        101,
        114,
        32,
        116,
        104,
        101,
        32,
        108,
        97,
        122,
        121,
        32,
        100,
        111,
        103,
    };

    try std.testing.expectEqual(text.len, textU32.len);
    try std.testing.expectEqual(textU32.len, textU32X.len);

    for (textU32, 0..) |value, i| {
        try std.testing.expectEqual(value, textU32X[i]);
    }

    al.free(textU32);
}

test "test utils->u8tou32->bengali" {
    const al = std.testing.allocator;
    const text = "এ বিশ্বকে এ শিশুর বাসযোগ্য করে যাব আমি\nনবজাতকের কাছে এ আমার দৃঢ় অঙ্গীকার";
    const textU32 = try _u8tou32(text, al);

    const textU32X = [_]u32{
        0x098f,
        0x0020,
        0x09ac,
        0x09bf,
        0x09b6,
        0x09cd,
        0x09ac,
        0x0995,
        0x09c7,
        0x0020,
        0x098f,
        0x0020,
        0x09b6,
        0x09bf,
        0x09b6,
        0x09c1,
        0x09b0,
        0x0020,
        0x09ac,
        0x09be,
        0x09b8,
        0x09af,
        0x09cb,
        0x0997,
        0x09cd,
        0x09af,
        0x0020,
        0x0995,
        0x09b0,
        0x09c7,
        0x0020,
        0x09af,
        0x09be,
        0x09ac,
        0x0020,
        0x0986,
        0x09ae,
        0x09bf,
        0x000a,
        0x09a8,
        0x09ac,
        0x099c,
        0x09be,
        0x09a4,
        0x0995,
        0x09c7,
        0x09b0,
        0x0020,
        0x0995,
        0x09be,
        0x099b,
        0x09c7,
        0x0020,
        0x098f,
        0x0020,
        0x0986,
        0x09ae,
        0x09be,
        0x09b0,
        0x0020,
        0x09a6,
        0x09c3,
        0x09a2,
        0x09bc,
        0x0020,
        0x0985,
        0x0999,
        0x09cd,
        0x0997,
        0x09c0,
        0x0995,
        0x09be,
        0x09b0,
    };

    try std.testing.expectEqual(textU32X.len, textU32.len);

    for (textU32, 0..) |value, i| {
        try std.testing.expectEqual(value, textU32X[i]);
    }

    al.free(textU32);
}

test "test utils->u32tou8->english" {
    const al = std.testing.allocator;
    const textU32 = [_]u32{
        84,
        104,
        101,
        32,
        113,
        117,
        105,
        99,
        107,
        32,
        98,
        114,
        111,
        119,
        110,
        32,
        102,
        111,
        120,
        32,
        106,
        117,
        109,
        112,
        115,
        32,
        111,
        118,
        101,
        114,
        32,
        116,
        104,
        101,
        32,
        108,
        97,
        122,
        121,
        32,
        100,
        111,
        103,
    };

    const textU8 = try _u32tou8(&textU32, al);

    const text = "The quick brown fox jumps over the lazy dog";

    try std.testing.expectEqual(textU8.len, textU8.len);

    for (textU8, 0..) |value, i| {
        try std.testing.expectEqual(value, text[i]);
    }

    al.free(textU8);
}
