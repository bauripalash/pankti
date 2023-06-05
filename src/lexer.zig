//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

const std = @import("std");
const utils = @import("utils.zig");
const bn = @import("bengali/bn.zig");

pub const TokenType = enum(u8) {
    Plus,
    Minus,
    Astr,
    Div,
    Eof,
    Lparen,
    Rparen,
    Equal,
    NotEqual,
    Semicolon,

    Number,

    Unknown,
};

pub fn toktypeToString(t: TokenType) []const u8 {
    return switch (t) {
        .Plus => "Plus",
        .Minus => "Minus",
        .Astr => "Astr",
        .Div => "Div",
        .Eof => "Eof",
        .Lparen => "Lparen",
        .Rparen => "Rparen",
        .Equal => "Equal",
        .NotEqual => "NotEqual",
        .Semicolon => "Semicolon",
        .Number => "Number",
        .Unknown => "Unknown",
    };
}

pub const Token = struct { lexeme: []const u32, toktype: TokenType, length: u32, colpos: u32, line: u32 };

pub const Lexer = struct {
    src: []u32,
    start: u32,
    current: u32,
    line: u32,
    colpos: u32,

    pub fn new(src: []u32) Lexer {
        return Lexer{
            .src = src,
            .start = 0,
            .line = 1,
            .colpos = 1,
            .current = 0,
        };
    }

    fn next(self: *Lexer) u32 {
        self.colpos += 1;
        self.current += 1;
        return self.src[self.current - 1];
    }

    fn peek(self: *Lexer) u32 {
        return self.src[self.current];
    }

    fn peekNext(self: *Lexer) u32 {
        if (self.isEof()) {
            return 0;
        }
        return self.src[self.current + 1];
    }

    pub fn isEof(self: *Lexer) bool {
        return self.current >= self.src.len;
    }

    fn makeToken(self: *Lexer, tt: TokenType) Token {
        return Token{
            .toktype = tt,
            .lexeme = self.src[self.start..self.current],
            .length = 0,
            .colpos = self.colpos,
            .line = self.line,
        };
    }

    fn skipWs(self: *Lexer) void {
        while (!self.isEof()) {
            const c = self.peek();

            switch (c) {
                ' ', '\r', '\t' => {
                    _ = self.next();
                },
                '\n' => {
                    self.line += 1;
                    self.colpos = 1;
                    _ = self.next();
                },

                '#' => {
                    while (self.peek() != '\n' and !self.isEof()) {
                        _ = self.next();
                    }
                },

                else => {
                    return;
                },
            }
        }
    }

    pub fn getToken(self: *Lexer) Token {
        self.skipWs();

        self.start = self.current;
        if (self.isEof()) {
            return self.makeToken(TokenType.Eof);
        }

        const c = self.next();

        if (bn.isBnNumber(c) or utils.isEnNum(c)) {
            return self.readNumberToken();
        }

        switch (c) {
            '+' => return self.makeToken(.Plus),
            '-' => return self.makeToken(.Minus),
            '*' => return self.makeToken(.Astr),
            '/' => return self.makeToken(.Div),
            '(' => return self.makeToken(.Lparen),
            ')' => return self.makeToken(.Rparen),
            ';' => return self.makeToken(.Semicolon),
            '=' => return self.makeToken(.Equal),
            else => {},
        }

        return self.makeToken(TokenType.Unknown);
    }

    pub fn readNumberToken(self: *Lexer) Token {
        while (bn.isBnNumber(self.peek()) or utils.isEnNum(self.peek())) {
            _ = self.next();
        }

        return self.makeNumberToken();
    }
    pub fn makeNumberToken(self: *Lexer) Token {
        return Token{
            .toktype = .Number,
            .lexeme = self.src[self.start..self.current],
            .line = self.line,
            .colpos = self.colpos,
            .length = 0,
        };
    }

    pub fn debug(self: *Lexer) void {
        while (!self.isEof()) {
            const tok = self.getToken();
            std.debug.print("T['", .{});
            utils.printu32(tok.lexeme);
            std.debug.print("'|{s}]\n", .{toktypeToString(tok.toktype)});
        }
    }
};
