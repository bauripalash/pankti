const std = @import("std");
const Allocator = std.mem.Allocator;

inline fn isdig(c: u8) bool {
    return c <= '9' and c >= '0';
}

/// BauriNum
pub const Bnum = struct {
    len: u64,
    sign: BnSign,
    digits: std.ArrayListUnmanaged(u4),

    const Self = @This();

    pub const BnSign = enum(u2) { Zero, Pos, Neg };
    pub const BnComp = enum(u2) { Lt, Gt, Eq };

    /// Creates a new Bnum;
    /// Check for errors!
    /// .len == 0
    ///.sign == Positive
    //
    ///free with .free() function
    pub fn new(al: Allocator) !*Bnum {
        const b = try al.create(Bnum);
        b.digits = std.ArrayListUnmanaged(u4){};
        b.len = 0;
        b.sign = .Pos;
        return b;
    }

    /// Returns true if the sign is either Positive or Zero
    pub fn isPos(self: *Self) bool {
        return (self.sign == BnSign.Pos or self.sign == BnSign.Zero);
    }

    fn trim(self: *Self) void {
        while (self.len > 0 and self.digits.items[self.len - 1] == 0) {
            self.len -= 1;
            _ = self.digits.pop();
        }

        if (self.len == 0) {
            self.sign = .Zero;
        }
    }

    pub fn print(self: *Self, showsign: bool) void {
        if (self.len <= 0) {
            return;
        }

        if (showsign) {
            if (self.isPos()) {
                std.debug.print("+", .{});
            } else {
                std.debug.print("-", .{});
            }
        }

        var i: isize = @intCast(self.len - 1);
        while (i >= 0) : (i -= 1) {
            std.debug.print("{d}", .{self.digits.items[@intCast(i)]});
        }

        std.debug.print("\n", .{});
    }

    pub fn toU64(self: *Self) u64 {
        var i: usize = 0;
        var k: u64 = 0;

        while (i < self.len) : (i += 1) {
            k = 10 * k + @as(u64, self.digits.items[i]);
        }
        return k;
    }

    pub fn toI64(self: *Self) i64 {
        const result = self.toU64();

        if (!self.isPos()) {
            return -@as(i64, @intCast(result));
        } else {
            return @intCast(result);
        }
    }

    pub fn toString(self: *Self, al: Allocator, showsign: bool) ?[]u8 {
        var totallen: usize = self.len;
        if (showsign) {
            totallen += 1;
        }

        const str = al.alloc(u8, totallen) catch return null;
        var ptr = str.ptr;

        if (showsign) {
            if (self.sign == .Neg) {
                ptr[0] = '-';
            } else {
                ptr[0] = '+';
            }

            ptr += 1;
        }

        var i: isize = @intCast(self.len - 1);

        while (i >= 0) : (i -= 1) {
            const x: u8 = @as(u8, @intCast(self.digits.items[@intCast(i)])) + '0';
            ptr[0] = x;
            ptr += 1;
        }

        return str;
    }

    ///Add a single digit;
    ///Increase the length;
    ///Returns true if everything went okay otherwise returns false;
    pub fn addDig(self: *Self, al: Allocator, digit: u8) bool {
        self.digits.append(al, @as(u4, @truncate(digit))) catch return false;
        self.len += 1;
        return true;
    }

    ///Set u64.
    ///Doesn't modify sign
    ///Return true if everything went okay otherwise returns false;
    pub fn setu64(self: *Self, al: Allocator, value: u64) bool {
        var t = value;
        while (t > 0) {
            if (!self.addDig(al, @as(u8, @truncate(@rem(t, 10))))) {
                return false;
            }

            t /= 10;
        }

        return true;
    }

    ///Set i64.
    ///Sets sign
    pub fn seti64(self: *Self, al: Allocator, value: i64) bool {
        if (value < 0) {
            self.sign = .Neg;
        } else {
            self.sign = .Pos;
        }

        // @abs was std.math.absCast
        return self.setu64(al, @intCast(@abs(value)));
    }

    pub fn reverseItems(self: *Self) void {
        var start: usize = 0;
        var end = self.len - 1;

        while (start < end) {
            const e = self.digits.items[end];
            const s = self.digits.items[start];
            self.digits.items[start] = e;
            self.digits.items[end] = s;
            start += 1;
            end -= 1;
        }
    }

    /// Set string.
    pub fn setstr(self: *Self, al: Allocator, value: []const u8) bool {
        var ptr = value.ptr;

        if (ptr[0] == '+') {
            self.sign = .Pos;
            ptr += 1;
        } else if (ptr[0] == '-') {
            self.sign = .Neg;
            ptr += 1;
        } else {
            self.sign = .Pos;
        }

        while ((@intFromPtr(ptr) - @intFromPtr(value.ptr)) < value.len) {
            if (isdig(ptr[0])) {
                if (!self.addDig(al, @as(u4, @truncate(ptr[0] - '0')))) {
                    return false;
                }
            } else if (ptr[0] == '.') {
                // Do nothing
            } else {
                return false;
            }
            ptr += 1;
        }

        self.reverseItems();

        return true;
    }

    pub fn ucomp(self: *Self, other: *Self) BnComp {
        if (self.len > other.len) {
            return .Gt;
        } else if (self.len < other.len) {
            return .Lt;
        }

        var i: isize = @intCast(self.len - 1);
        while (i >= 0) : (i -= 1) {
            if (self.digits.items[@intCast(i)] > other.digits.items[@intCast(i)]) {
                return .Gt;
            } else if (self.digits.items[@intCast(i)] < other.digits.items[@intCast(i)]) {
                return .Lt;
            }
        }

        return .Eq;
    }

    pub fn comp(self: *Self, other: *Self) BnComp {
        if (self.sign != other.sign) {
            if (!self.isPos()) {
                return .Lt;
            } else {
                return .Gt;
            }
        }

        if (!self.isPos()) {
            return other.ucomp(self);
        } else {
            return self.ucomp(other);
        }
    }

    pub fn add(self: *Self, other: *Self, al: Allocator) ?*Bnum {
        if (self.sign == other.sign) {
            const result = self._add(other, al) orelse return null;
            result.sign = self.sign;
            return result;
        } else if (self.ucomp(other) == .Lt) {
            const result = other._sub(self, al) orelse return null;
            result.sign = other.sign;
            return result;
        } else {
            const result = self._sub(other, al) orelse return null;
            result.sign = self.sign;
            return result;
        }

        return null;
    }

    pub fn sub(self: *Self, other: *Self, al: Allocator) ?*Bnum {
        if (self.sign != other.sign) {
            const result = self._add(other, al) orelse return null;
            result.sign = self.sign;
            return result;
        } else if ((self.ucomp(other) == .Gt) or (self.ucomp(other) == .Eq)) {
            const result = self._sub(other, al) orelse return null;
            result.sign = self.sign;
            return result;
        } else {
            const result = other._sub(self, al) orelse return null;
            if (self.sign == .Neg) {
                result.sign = .Pos;
            } else {
                result.sign = .Neg;
            }

            return result;
        }

        return null;
    }

    pub fn _sub(self: *Self, other: *Self, al: Allocator) ?*Bnum {
        var result = Bnum.new(al) catch return null;
        const minlen = other.len;
        const maxlen = self.len;

        result.len = maxlen;

        var c: u64 = 0;
        var i: usize = 0;

        while (i < minlen) : (i += 1) {
            var x: u64 = self.digits.items[i];
            x -= c;
            const y: u64 = other.digits.items[i];

            if (x < y) {
                x += 10;
                c = 1;
            } else {
                c = 0;
            }

            result.digits.append(al, @as(u4, @truncate(x - y))) catch return null;
        }

        while (i < maxlen) : (i += 1) {
            var x: u64 = self.digits.items[i];
            x -= c;
            c = x / 10;
            result.digits.append(al, @as(u4, @truncate(@mod(x, 10)))) catch return null;
        }

        result.trim();
        return result;
    }
    fn _add(self: *Self, other: *Self, al: Allocator) ?*Bnum {
        var result = Bnum.new(al) catch return null;
        var min: usize = 0;
        var max: usize = 0;
        var bgx: *Bnum = undefined;

        if (self.len > other.len) {
            min = other.len;
            max = self.len;
            bgx = self;
        } else {
            min = self.len;
            max = other.len;
            bgx = other;
        }

        const oldrlen = result.len;
        _ = oldrlen;
        result.len = max + 1;

        var u: u64 = 0;
        var i: usize = 0;

        while (i < min) : (i += 1) {
            //std.debug.print("-->{d}|{d}|{d}<--", .{
            //    self.digits.items[i],
            //    other.digits.items[i],
            //    u,
            //});
            var x: u64 = self.digits.items[i];
            x += other.digits.items[i];
            x += u;
            u = x / 10;
            //std.debug.print("->{d}|U{}\n", .{ x, u });

            result.digits.append(al, @as(u4, @truncate(@mod(x, 10)))) catch return null;
        }

        if (min != max) {
            while (i < max) : (i += 1) {
                var x: u64 = bgx.digits.items[i];
                x += u;
                u = x / 10;
                result.digits.append(al, @as(u4, @truncate(@mod(x, 10)))) catch return null;
            }
        }

        result.digits.append(al, @truncate(u)) catch return null;

        result.trim();
        //result.print(true);
        return result;
    }

    ///Free the Bnum
    pub fn free(self: *Self, al: Allocator) void {
        self.digits.deinit(al);
        al.destroy(self);
    }
};
