//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const utils = @import("../utils.zig");
const bn = @import("../bengali/bn.zig");
const kw = @import("keywords.zig");

pub const TokenType = enum(u8) {
    Plus,
    Minus,
    Astr,
    PowAstr,
    Slash,
    Mod,
    Eof,
    Lparen,
    Rparen,
    Lbrace,
    Rbrace,
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

    //keywords
    Let,
    //Show,
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
    Do,
    Break,

    Unknown,
    Err,
};

pub fn toktypeToString(t: TokenType) []const u8 {
    return switch (t) {
        .Plus => "Plus",
        .Minus => "Minus",
        .Astr => "Astr",
        .PowAstr => "PowAstr",
        .Slash => "Slash",
        .Mod => "Mod",
        .Eof => "Eof",
        .Lparen => "Lparen",
        .Rparen => "Rparen",
        .Lbrace => "Lbrace",
        .Rbrace => "Rbrace",
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
        //.Show => "Show",
        .And => "And",
        .Or => "Or",
        .End => "End",
        .If => "If",
        .Then => "Then",
        .Else => "Else",
        .PWhile => "While",
        .Do => "Do",
        .Nil => "Nil",
        .True => "True",
        .False => "False",
        .Return => "Return",
        .Func => "Func",
        .Import => "Import",
        .Panic => "Panic",
        .Break => "Break",
        .Unknown => "Unknown",
        .Err => "Error",
        //else => "INVALID"
    };
}

fn isValidNumber(c: u21) bool {
    return utils.isEnNum(c) or bn.isBnNumber(c);
}

fn isValidIdentChar(c: u21) bool {
    return utils.isValidEn(c) or bn.isBnChar(c);
}

pub const Token = struct {
    lexeme: []const u8,
    toktype: TokenType,
    length: usize,
    colpos: usize,
    line: usize,

    pub fn dummy() Token {
        return Token{
            .lexeme = "_",
            .toktype = .Err,
            .length = 1,
            .colpos = 1,
            .line = 1,
        };
    }

    pub fn toString(self: *const Token, al: std.mem.Allocator) ![]u8 {
        const result = try std.fmt.allocPrint(
            al,
            "T[{s}|{s}|]",
            .{ toktypeToString(self.toktype), self.lexeme },
        );
        return result;
    }
};

pub const Lexer = struct {
    src: []const u8,
    uview: std.unicode.Utf8View,
    it: std.unicode.Utf8Iterator,
    start: usize,
    current: usize,
    line: usize,
    colpos: usize,

    const Self = @This();

    pub fn dummy() Lexer {
        return Lexer{
            .src = undefined,
            .uview = undefined,
            .it = undefined,
            .start = 0,
            .line = 1,
            .colpos = 1,
            .current = 0,
        };
    }

    pub fn new(src: []const u8) Lexer {
        var lx = Lexer{
            .src = src,
            .uview = undefined,
            .it = undefined,
            .start = 0,
            .line = 1,
            .colpos = 1,
            .current = 0,
        };

        lx.uview = std.unicode.Utf8View.initUnchecked(lx.src);
        lx.it = lx.uview.iterator();

        return lx;
    }

    fn next(self: *Lexer) u21 {
        const j = self.it.i;
        if (self.it.nextCodepoint()) |cp| {
            self.colpos += 1; //It's kinda wrong
            self.current += (self.it.i - j);
            return cp;
        }
        return 0;
    }

    fn _peek(self: *Self, n: usize) u21 {
        if (self.isEof()) return 0;
        if (n < 0) return 0;
        const raw = self.it.peek(n);
        if (raw.len <= 0) return 0;
        const s = raw[n - 1 ..];
        return std.unicode.utf8Decode(s) catch return 0;

        // if (n == 1) {
        //     const raw = self.it.peek(n);
        //     return std.unicode.utf8Decode(raw) catch return 0;
        // } else if (n == 2) {
        //     const raw = self.it.peek(n);
        //
        //     if (raw.len < 2) {
        //         return 0;
        //     }
        //
        //     const s = raw[1..];
        //     return std.unicode.utf8Decode(s) catch return 0;
        // }
        //
        // return 0;
        // if (self.isEof()) return 0x04;
        // std.debug.print("\n\n->>{d}|'{u}'\n\n", .{ n, self.it.peek(n) });
        // const result = std.unicode.utf8Decode(self.it.peek(n)) catch {
        //     return 0;
        // };
        //
        // return result;
    }

    fn peek(self: *Lexer) u21 {
        //if (self.isEof()) return 0x04;
        //return self.src[self.current];
        return self._peek(1);
    }

    fn peekNext(self: *Lexer) u21 {
        //if (self.isEof()) {
        //    return 0;
        //}
        //return self.src[self.current + 1];
        return self._peek(2);
    }

    pub fn isEof(self: *Lexer) bool {
        //return self.current >= self.src.len;
        return self.it.i >= self.it.bytes.len;
    }

    fn makeToken(self: *Lexer, tt: TokenType, single: bool) Token {
        return Token{
            .toktype = tt,
            .lexeme = self.src[self.start..self.current],
            .length = if (single) 1 else (self.current - self.start),
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

    fn matchChar(self: *Self, c: u21) bool {
        if (self.isEof()) {
            return false;
        }
        if (self.peek() != c) {
            return false;
        }
        //self.current += 1;
        _ = self.next();
        return true;
    }

    pub fn getToken(self: *Lexer) Token {
        self.skipWs();

        self.start = self.current;
        if (self.isEof()) {
            return self.makeToken(TokenType.Eof, true);
        }

        const c = self.next();

        if (bn.isBnNumber(c) or utils.isEnNum(c)) {
            return self.readNumberToken();
        }

        if (isValidIdentChar(c)) {
            return self.readIdentifierToken();
        }

        switch (c) {
            '(' => return self.makeToken(.Lparen, true),
            ')' => return self.makeToken(.Rparen, true),
            '{' => return self.makeToken(.Lbrace, true),
            '}' => return self.makeToken(.Rbrace, true),
            '[' => return self.makeToken(.LsBracket, true),
            ']' => return self.makeToken(.RsBracket, true),
            '-' => return self.makeToken(.Minus, true),
            '+' => return self.makeToken(.Plus, true),
            '/' => return self.makeToken(.Slash, true),
            '%' => return self.makeToken(.Mod, true),
            '*' => {
                if (self.matchChar('*')) {
                    return self.makeToken(.PowAstr, false);
                } else {
                    return self.makeToken(.Astr, false);
                }
            },
            '.' => return self.makeToken(.Dot, true),
            ';' => return self.makeToken(.Semicolon, true),
            ':' => return self.makeToken(.Colon, true),
            ',' => return self.makeToken(.Comma, true),
            '"' => return self.readStringToken(),

            '!' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.NotEqual, false);
                } else {
                    return self.makeToken(.Bang, true);
                }
            },

            '=' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.EqEq, false);
                } else {
                    return self.makeToken(.Eq, true);
                }
            },

            '>' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.Gte, false);
                } else {
                    return self.makeToken(.Gt, true);
                }
            },

            '<' => {
                if (self.matchChar('=')) {
                    return self.makeToken(.Lte, false);
                } else {
                    return self.makeToken(.Lt, true);
                }
            },

            else => {
                return self.makeToken(TokenType.Unknown, true);
            },
        }

        return self.makeToken(TokenType.Unknown, true);
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

    fn makeStringToken(self: *Self, colpos: usize, line: usize) Token {
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
    fn makeNumberToken(self: *Lexer, colpos: usize) Token {
        const lexeme = self.src[self.start..self.current];
        return Token{
            .toktype = .Number,
            .lexeme = lexeme,
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

    fn getIdentifierType(_: *Self, lx: []const u8) TokenType {
        if (utils.matchIdent(
            lx,
            kw.K_EN_LET,
            kw.K_PN_LET,
            kw.K_BN_LET,
        )) {
            return TokenType.Let;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_AND,
            kw.K_PN_AND,
            kw.K_BN_AND,
        )) {
            return TokenType.And;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_OR,
            kw.K_PN_OR,
            kw.K_BN_OR,
        )) {
            return TokenType.Or;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_END,
            kw.K_PN_END,
            kw.K_BN_END,
        )) {
            return TokenType.End;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_IF,
            kw.K_PN_IF,
            kw.K_BN_IF,
        )) {
            return TokenType.If;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_THEN,
            kw.K_PN_THEN,
            kw.K_BN_THEN,
        )) {
            return TokenType.Then;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_ELSE,
            kw.K_PN_ELSE,
            kw.K_BN_ELSE,
        )) {
            return TokenType.Else;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_WHILE,
            kw.K_PN_WHILE,
            kw.K_BN_WHILE,
        )) {
            return TokenType.PWhile;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_NIL,
            kw.K_PN_NIL,
            kw.K_BN_NIL,
        )) {
            return TokenType.Nil;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_TRUE,
            kw.K_PN_TRUE,
            kw.K_BN_TRUE,
        )) {
            return TokenType.True;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_FALSE,
            kw.K_PN_FALSE,
            kw.K_BN_FALSE,
        )) {
            return TokenType.False;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_RETURN,
            kw.K_PN_RETURN,
            kw.K_BN_RETURN,
        )) {
            return TokenType.Return;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_FUNC,
            kw.K_PN_FUNC,
            kw.K_BN_FUNC,
        )) {
            return TokenType.Func;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_IMPORT,
            kw.K_PN_IMPORT,
            kw.K_BN_IMPORT,
        )) {
            return TokenType.Import;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_PANIC,
            kw.K_PN_PANIC,
            kw.K_BN_PANIC,
        )) {
            return TokenType.Panic;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_DO,
            kw.K_PN_DO,
            kw.K_BN_DO,
        )) {
            return TokenType.Do;
        } else if (utils.matchIdent(
            lx,
            kw.K_EN_BREAK,
            kw.K_PN_BREAK,
            kw.K_BN_BREAK,
        )) {
            return TokenType.Break;
        }

        return TokenType.Identifer;
    }

    fn makeIdentifierToken(self: *Self, colpos: usize) Token {
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
            std.io.getStdOut().print("T['{s}'|{d}|{s}]\n", .{
                tok.lexeme,
                tok.length,
                toktypeToString(tok.toktype),
            }) catch return;
        }
    }
};

fn tx(t: TokenType) Token {
    return Token{
        .lexeme = &[_]u8{'1'},
        .toktype = t,
        .length = 0,
        .colpos = 0,
        .line = 0,
    };
}

test "lexer TokenType testing" {
    const rawSource = "let name = 1; show(1+2);";
    var Lx = Lexer.new(rawSource);

    const toks: []const Token = &[_]Token{
        tx(.Let),
        tx(.Identifer),
        tx(.Eq),
        tx(.Number),
        tx(.Semicolon),
        tx(.Identifer),
        tx(.Lparen),
        tx(.Number),
        tx(.Plus),
        tx(.Number),
        tx(.Rparen),
        tx(.Semicolon),
        tx(.Eof),
    };

    for (toks) |t| {
        const tr = Lx.getToken();
        try std.testing.expectEqual(t.toktype, tr.toktype);
    }
}

test "lexer double char tokens" {
    const src = "let a = 2 ** 4; let b = a != 0; ";
    var lx = Lexer.new(src);

    const toks: []const Token = &[_]Token{
        tx(.Let),
        tx(.Identifer),
        tx(.Eq),
        tx(.Number),
        tx(.PowAstr),
        tx(.Number),
        tx(.Semicolon),
        tx(.Let),
        tx(.Identifer),
        tx(.Eq),
        tx(.Identifer),
        tx(.NotEqual),
        tx(.Number),
        tx(.Semicolon),
    };

    for (toks) |t| {
        const tk = lx.getToken();
        try std.testing.expectEqual(t.toktype, tk.toktype);
    }
}

test "lexer bengali keywords" {
    const src = "ধরি নাম = ১০০**২০০";
    var lx = Lexer.new(src);

    const toks = &[_]Token{
        tx(.Let),
        tx(.Identifer),
        tx(.Eq),
        tx(.Number),
        tx(.PowAstr),
        tx(.Number),
    };

    for (toks) |t| {
        const tk = lx.getToken();
        //std.debug.print("{any}->{any}\n", .{ t.toktype, tk.toktype });
        try std.testing.expectEqual(t.toktype, tk.toktype);
    }
}
