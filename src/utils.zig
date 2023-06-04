const std = @import("std");

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
