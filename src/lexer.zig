const std = @import("std");

pub const TokenType = enum(u8) {
    Plus,
    Minus,
    Astr,
    Div,
};

pub const Token = struct { start: [*]const u32, toktype: TokenType, length: u32, colpos: u32, line: u32 };

pub const Lexer = struct {
    start: []u32,
    current: []u32,
    line: u32,
    colpos: u32,

    pub fn new(src: []u32) Lexer {
        return Lexer{
            .start = src,
            .line = 1,
            .colpos = 1,
            .current = src
    };
    }

    pub fn debug(self : *Lexer) void {
        for (self.*.start) |value| {
            
            std.debug.print("->{u}<-\n", .{@truncate(u21 , value)});

        }
    }
};
