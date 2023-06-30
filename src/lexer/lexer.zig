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
        .Unknown => "Unknown",
        .Err => "Error",
        //else => "INVALID"
    };
}

fn isValidNumber(c: u32) bool {
    return utils.isEnNum(c) or bn.isBnNumber(c);
}

fn isValidIdentChar(c: u32) bool {
    return utils.isValidEn(c) or bn.isBnChar(c);
}

pub const Token = struct {
    lexeme: []const u32,
    toktype: TokenType,
    length: u32,
    colpos: u32,
    line: u32,

    pub fn dummy() Token {
        return Token{
            .lexeme = &[_]u32{'_'},
            .toktype = .Err,
            .length = 1,
            .colpos = 1,
            .line = 1,
        };
    }

    pub fn toString(self: *const Token, al: std.mem.Allocator) ![]u8 {
        const lexeme = try utils.u32tou8(self.lexeme, al);
        const result = try std.fmt.allocPrint(
            al,
            "T[{s}|{s}|]",
            .{ toktypeToString(self.toktype), lexeme },
        );
        al.free(lexeme);
        return result;
    }
};

pub const Lexer = struct {
    src: []u32,
    start: u32,
    current: u32,
    line: u32,
    colpos: u32,

    const Self = @This();

    pub fn dummy() Lexer {
        return Lexer{
            .src = undefined,
            .start = 0,
            .line = 1,
            .colpos = 1,
            .current = 0,
        };
    }

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
        if (self.isEof()) return 0x04;
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
            '{' => return self.makeToken(.Lbrace),
            '}' => return self.makeToken(.Rbrace),
            '[' => return self.makeToken(.LsBracket),
            ']' => return self.makeToken(.RsBracket),
            '-' => return self.makeToken(.Minus),
            '+' => return self.makeToken(.Plus),
            '/' => return self.makeToken(.Slash),
            '*' => {
                if (self.matchChar('*')) {
                    return self.makeToken(.PowAstr);
                } else {
                    return self.makeToken(.Astr);
                }
            },
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

        var lexeme = self.src[self.start..self.current];
        var i : usize = 0;
        while (i < lexeme.len) : (i += 1) {
            switch (lexeme[i]) {
               bn.BN_NUM_0 => lexeme[i] = '0',
               bn.BN_NUM_1 => lexeme[i] = '1',
               bn.BN_NUM_2 => lexeme[i] = '2',
               bn.BN_NUM_3 => lexeme[i] = '3',
               bn.BN_NUM_4 => lexeme[i] = '4',
               bn.BN_NUM_5 => lexeme[i] = '5',
               bn.BN_NUM_6 => lexeme[i] = '6',
               bn.BN_NUM_7 => lexeme[i] = '7',
               bn.BN_NUM_8 => lexeme[i] = '8',
               bn.BN_NUM_9 => lexeme[i] = '9',
               else => {},
            }        
        }

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

    fn getIdentifierType(_: *Self, lx: []u32) TokenType {
        if (utils.matchU32(lx, &kw.K_EN_LET) or utils.matchU32(lx, &kw.K_BN_LET) or utils.matchU32(lx, &kw.K_PN_LET)) {
            return .Let;
            // } else if (utils.matchU32(lx, &kw.K_EN_SHOW)
            //     or utils.matchU32(lx, &kw.K_BN_SHOW)
            //     or utils.matchU32(lx, &kw.K_PN_SHOW) ) {
            //     return .Show;
        } else if (utils.matchU32(lx, &kw.K_EN_AND) or utils.matchU32(lx, &kw.K_BN_AND) or utils.matchU32(lx, &kw.K_PN_AND)) {
            return .And;
        } else if (utils.matchU32(lx, &kw.K_EN_OR) or utils.matchU32(lx, &kw.K_BN_OR) or utils.matchU32(lx, &kw.K_PN_OR)) {
            return .Or;
        } else if (utils.matchU32(lx, &kw.K_EN_END) or utils.matchU32(lx, &kw.K_BN_END) or utils.matchU32(lx, &kw.K_PN_END)) {
            return .End;
        } else if (utils.matchU32(lx, &kw.K_EN_IF) or utils.matchU32(lx, &kw.K_BN_IF) or utils.matchU32(lx, &kw.K_PN_IF)) {
            return .If;
        } else if (utils.matchU32(lx, &kw.K_EN_THEN) or utils.matchU32(lx, &kw.K_BN_THEN) or utils.matchU32(lx, &kw.K_PN_THEN)) {
            return .Then;
        } else if (utils.matchU32(lx, &kw.K_EN_ELSE) or utils.matchU32(lx, &kw.K_BN_ELSE) or utils.matchU32(lx, &kw.K_PN_ELSE)) {
            return .Else;
        } else if (utils.matchU32(lx, &kw.K_EN_WHILE) or utils.matchU32(lx, &kw.K_BN_WHILE) or utils.matchU32(lx, &kw.K_PN_WHILE)) {
            return .PWhile;
        } else if (utils.matchU32(lx, &kw.K_EN_NIL) or utils.matchU32(lx, &kw.K_BN_NIL) or utils.matchU32(lx, &kw.K_PN_NIL)) {
            return .Nil;
        } else if (utils.matchU32(lx, &kw.K_EN_TRUE) or utils.matchU32(lx, &kw.K_BN_TRUE) or utils.matchU32(lx, &kw.K_PN_TRUE)) {
            return .True;
        } else if (utils.matchU32(lx, &kw.K_EN_FALSE) or utils.matchU32(lx, &kw.K_BN_FALSE) or utils.matchU32(lx, &kw.K_PN_FALSE)) {
            return .False;
        } else if (utils.matchU32(lx, &kw.K_EN_RETURN) or utils.matchU32(lx, &kw.K_BN_RETURN) or utils.matchU32(lx, &kw.K_PN_RETURN)) {
            return .Return;
        } else if (utils.matchU32(lx, &kw.K_EN_FUNC) or utils.matchU32(lx, &kw.K_BN_FUNC) or utils.matchU32(lx, &kw.K_PN_FUNC)) {
            return .Func;
        } else if (utils.matchU32(lx, &kw.K_EN_IMPORT) or utils.matchU32(lx, &kw.K_BN_IMPORT) or utils.matchU32(lx, &kw.K_PN_IMPORT)) {
            return .Import;
        } else if (utils.matchU32(lx, &kw.K_EN_PANIC) or utils.matchU32(lx, &kw.K_BN_PANIC) or utils.matchU32(lx, &kw.K_PN_PANIC)) {
            return .Panic;
        } else if (utils.matchU32(lx, &kw.K_EN_DO) or utils.matchU32(lx, &kw.K_BN_DO) or utils.matchU32(lx, &kw.K_PN_DO)) {
            return .Do;
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
            std.debug.print("'|{}|{s}]\n", .{
                tok.length,
                toktypeToString(tok.toktype),
            });
        }
    }
};

fn tx(t: TokenType) Token {
    return Token{
        .lexeme = &[_]u32{'1'},
        .toktype = t,
        .length = 0,
        .colpos = 0,
        .line = 0,
    };
}

test "lexer TokenType testing" {
    const rawSource = "show(1+2);";
    var a = std.testing.allocator;
    const source: []u32 = try utils.u8tou32(rawSource, a);
    var Lx = Lexer.new(source);

    var toks: []const Token = &[_]Token{
        tx(.Identifer),
        tx(.Lparen),
        tx(.Number),
        tx(.Plus),
        tx(.Number),
        tx(.Rparen),
        tx(.Semicolon),
    };

    for (toks) |t| {
        const tr = Lx.getToken();
        try std.testing.expectEqual(t.toktype, tr.toktype);
    }

    a.free(source);
}
