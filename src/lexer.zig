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
const kw = @import("keywords.zig");

pub const TokenType = enum(u8) {
    Plus,
    Minus,
    Astr,
    Div,
    Eof,
    Lparen,
    Rparen,
    Lbracket,
    Rbracket,
    LsBracket,
    RsBracket,
    Dot,

    Semicolon,
    Colon,

    Comma,
    Bang,

    Eq,
    EqEq,
    NotEqual,

    Gt,
    Gte,
    Lt,
    Lte,

    Number,
    Identifer,
    String,

    Let,
    Show,
    And,
    Or,
    End,
    If,
    Then,
    Else,
    PWhile,
    Nil,
    True,
    False,
    Return,
    Func,
    Import,
    Panic,

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
        .Lbracket => "Lbracket",
        .Rbracket => "Rbracket",
        .LsBracket => "LsBracket",
        .RsBracket => "RsBracket",
        .Dot => "Dot",
        .Semicolon => "Semicolon",
        .Colon => "Colon",
        .Comma => "Comma",
        .Bang => "Bang",
        .Eq => "Eq",
        .EqEq => "EqEq",
        .NotEqual => "NotEqual",
        .Gt => "Gt",
        .Gte => "Gte",
        .Lt => "Lt",
        .Lte => "Lte",
        .Number => "Number",
        .Identifer => "Identifer",
        .String => "String",
        .Let => "Let",
        .Show => "Show",
        .And => "And",
        .Or => "Or",
        .End => "End",
        .If => "If",
        .Then => "Then",
        .Else => "Else",
        .PWhile => "While",
        .Nil => "Nil",
        .True => "True",
        .False => "False",
        .Return => "Return",
        .Func => "Func",
        .Import => "Import",
        .Panic => "Panic",
        .Unknown => "Unknown",
        //else => "INVALID"
    };
}

fn isValidNumber(c: u32) bool {
    return utils.isEnNum(c) or bn.isBnNumber(c);
}

fn isValidIdentChar(c: u32) bool {
    return utils.isValidEn(c) or bn.isBnChar(c);
}

pub const Token = struct { lexeme: []const u32, toktype: TokenType, length: u32, colpos: u32, line: u32 };

pub const Lexer = struct {
    src: []u32,
    start: u32,
    current: u32,
    line: u32,
    colpos: u32,

    const Self = @This();

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
            .length = self.current - self.start,
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

    fn matchChar(self: *Self, c: u32) bool {
        if (self.isEof()) {
            return false;
        }
        if (self.src[self.current] != c) {
            return false;
        }
        self.current += 1;
        return true;
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

        if (isValidIdentChar(c)) {
            return self.readIdentifierToken();
        }

        switch (c) {
            '(' => return self.makeToken(.Lparen),
            ')' => return self.makeToken(.Rparen),
            '{' => return self.makeToken(.Lbracket),
            '}' => return self.makeToken(.Rbracket),
            '[' => return self.makeToken(.LsBracket),
            ']' => return self.makeToken(.RsBracket),
            '-' => return self.makeToken(.Minus),
            '+' => return self.makeToken(.Plus),
            '/' => return self.makeToken(.Div),
            '*' => return self.makeToken(.Astr),
            '.' => return self.makeToken(.Dot),
            ';' => return self.makeToken(.Semicolon),
            ':' => return self.makeToken(.Colon),
            ',' => return self.makeToken(.Comma),
            '"' => return self.readStringToken(),

            '!' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.NotEqual);
                } else {
                    return self.makeToken(.Bang);
                }
            },

            '=' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.EqEq);
                } else {
                    return self.makeToken(.Eq);
                }
            },

            '>' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.Gte);
                } else {
                    return self.makeToken(.Gt);
                }
            },

            '<' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.Lte);
                } else {
                    return self.makeToken(.Lt);
                }
            },

            else => {
                return self.makeToken(TokenType.Unknown);
            },
        }

        return self.makeToken(TokenType.Unknown);
    }

    fn readStringToken(self: *Self) Token {
        const colpos = self.colpos;
        const line = self.line;
        while (self.peek() != '"' and !self.isEof()) {
            if (self.peek() == '\n') {
                self.line += 1;
                self.colpos = 1;
            } else if (self.peek() == '\\' and !self.isEof()) {
                _ = self.next();
            }

            _ = self.next();
        }

        _ = self.next();
        return self.makeStringToken(colpos, line);
    }

    fn makeStringToken(self: *Self, colpos: u32, line: u32) Token {
        return Token{
            .lexeme = self.src[self.start..self.current],
            .toktype = .String,
            .length = self.current - self.start,
            .line = line,
            .colpos = colpos,
        };
    }

    fn readNumberToken(self: *Lexer) Token {
        const colpos = self.colpos;
        while (isValidNumber(self.peek())) {
            _ = self.next();
        }

        if (self.peek() == '.' and isValidNumber(self.peekNext())) {
            _ = self.next();
            while (isValidNumber(self.peek())) {
                _ = self.next();
            }
        }

        return self.makeNumberToken(colpos);
    }
    fn makeNumberToken(self: *Lexer, colpos: u32) Token {
        return Token{
            .toktype = .Number,
            .lexeme = self.src[self.start..self.current],
            .line = self.line,
            .colpos = colpos,
            .length = self.current - self.start,
        };
    }

    fn readIdentifierToken(self: *Self) Token {
        const colpos = self.colpos;
        while (isValidIdentChar(self.peek()) or isValidNumber(self.peek())) {
            _ = self.next();
        }

        return self.makeIdentifierToken(colpos);
    }

    fn getIdentifierType(_: *Self, lx: []u32) TokenType {
        if (utils.matchU32(lx, &kw.K_EN_LET) or utils.matchU32(lx, &kw.K_BN_LET)) {
            return .Let;
        } else if (utils.matchU32(lx, &kw.K_EN_SHOW) or utils.matchU32(lx, &kw.K_BN_SHOW)) {
            return .Show;
        } else if (utils.matchU32(lx, &kw.K_EN_AND) or utils.matchU32(lx, &kw.K_BN_AND)) {
            return .And;
        } else if (utils.matchU32(lx, &kw.K_EN_OR) or utils.matchU32(lx, &kw.K_BN_OR)) {
            return .Or;
        } else if (utils.matchU32(lx, &kw.K_EN_END) or utils.matchU32(lx, &kw.K_BN_END)) {
            return .End;
        } else if (utils.matchU32(lx, &kw.K_EN_IF) or utils.matchU32(lx, &kw.K_BN_IF)) {
            return .If;
        } else if (utils.matchU32(lx, &kw.K_EN_THEN) or utils.matchU32(lx, &kw.K_BN_THEN)) {
            return .Then;
        } else if (utils.matchU32(lx, &kw.K_EN_ELSE) or utils.matchU32(lx, &kw.K_BN_ELSE)) {
            return .Else;
        } else if (utils.matchU32(lx, &kw.K_EN_WHILE) or utils.matchU32(lx, &kw.K_BN_WHILE)) {
            return .PWhile;
        } else if (utils.matchU32(lx, &kw.K_EN_NIL) or utils.matchU32(lx, &kw.K_BN_NIL)) {
            return .Nil;
        } else if (utils.matchU32(lx, &kw.K_EN_TRUE) or utils.matchU32(lx, &kw.K_BN_TRUE)) {
            return .True;
        } else if (utils.matchU32(lx, &kw.K_EN_FALSE) or utils.matchU32(lx, &kw.K_BN_FALSE)) {
            return .False;
        } else if (utils.matchU32(lx, &kw.K_EN_RETURN) or utils.matchU32(lx, &kw.K_BN_RETURN)) {
            return .Return;
        } else if (utils.matchU32(lx, &kw.K_EN_FUNC) or utils.matchU32(lx, &kw.K_BN_FUNC)) {
            return .Func;
        } else if (utils.matchU32(lx, &kw.K_EN_IMPORT) or utils.matchU32(lx, &kw.K_BN_IMPORT)) {
            return .Import;
        } else if (utils.matchU32(lx, &kw.K_EN_PANIC) or utils.matchU32(lx, &kw.K_BN_PANIC)) {
            return .Panic;
        }
        return .Identifer;
    }

    fn makeIdentifierToken(self: *Self, colpos: u32) Token {
        const lexeme = self.src[self.start..self.current];
        return Token{
            .toktype = self.getIdentifierType(lexeme),
            .lexeme = lexeme,
            .line = self.line,
            .colpos = colpos,
            .length = self.current - self.start,
        };
    }

    pub fn debug(self: *Lexer) void {
        while (!self.isEof()) {
            const tok = self.getToken();
            std.debug.print("T['", .{});
            utils.printu32(tok.lexeme);
            std.debug.print("'|{}|{s}]\n", .{ tok.length, toktypeToString(tok.toktype) });
        }
    }
};
