//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0


const std = @import("std");
const utils = @import("utils.zig");
const lexer = @import("lexer/lexer.zig");
const ins = @import("instruction.zig");
const PValue = @import("value.zig").PValue;
const PObj = @import("object.zig").PObj;
const Vm = @import("vm.zig").Vm;

const DEBUG = true;

const Precedence = enum(u8) {
    P_None,
    P_Assignment,
    P_Or,
    P_And,
    P_Eq,
    P_Comp,
    P_Term,
    P_Fact,
    P_Unary,
    P_Call,
    P_Pry,

};

pub const Compiler = struct {
    parser : Parser,
    al : std.mem.Allocator,
    inst : *ins.Instruction,

    const Self = @This();

    const ParseRule = struct {
        prefix : ?*const fn (*Self , bool) anyerror!void = null,
        infix : ?*const fn (*Self , bool) anyerror!void = null,
        prec : Precedence = .P_None,
    };

    const RuleTable = std.EnumArray(lexer.TokenType, ParseRule);

    const rules = RuleTable.init(.{
        .Lparen = ParseRule{
            .prefix = Self.rGroup,
            .prec = .P_Call,
        },
        .Rparen = ParseRule{},
        .Lbrace  = ParseRule{},
        .Rbrace = ParseRule{},
        .LsBracket = ParseRule{},
        .RsBracket = ParseRule{},
        .Colon = ParseRule{},
        .End = ParseRule{},
        .Then = ParseRule{},
        .Func = ParseRule{},
        .Import = ParseRule{},
        .Panic = ParseRule{},
        .Unknown = ParseRule{},
        .Comma = ParseRule{},
        .Dot = ParseRule{},
        .Minus = ParseRule{
            .prefix = Self.rUnary,
            .infix = Self.rBinary,
            .prec = .P_Term,
        },
        .Plus = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Term, 
        },

        .Semicolon = ParseRule{},
        .Slash = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Fact,
        },

        .Astr = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Fact
        },
        .Bang = ParseRule{
            .prefix = Self.rUnary,
        },
        .NotEqual = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Eq
        },
        .Eq = ParseRule{ },
        .EqEq = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Eq,
        },
        .Gt = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Comp,
        },
        .Gte = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Comp,
        },
        .Lt = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Comp,
        },
        .Lte = ParseRule{
            .infix = Self.rBinary,
            .prec = .P_Comp,
        },
        .Identifer = ParseRule{},
        .String = ParseRule{
           .prefix = Self.rString, 
        },
        .Number = ParseRule{
            .prefix = Self.rNumber,
            .prec = .P_None,
        },
        .And = ParseRule{},
        .If = ParseRule{},
        .Else = ParseRule{},
        .True = ParseRule{.prefix = Self.rLiteral},
        .False = ParseRule{.prefix = Self.rLiteral},
        .Nil = ParseRule{.prefix = Self.rLiteral},
        .Or = ParseRule{},
        .Show = ParseRule{},
        .Return = ParseRule{},
        .Let = ParseRule{},
        .PWhile = ParseRule{},
        .Err = ParseRule{},
        .Eof = ParseRule{},
        
    });
    
    pub fn init(self : *Self , vm : *Vm) void{
        self.al = vm.al;
        self.parser  = Parser.new();
        self.parser.vm = vm;
    }

    fn emitBtRaw(self : *Self , bt : u8) !void{
        try self.inst.write_raw(bt, ins.InstPos.line(self.parser.previous.line));
    }

    fn emitBt(self : *Self , bt : ins.OpCode) !void{
        try self.inst.write(bt, ins.InstPos.line(self.parser.previous.line));
    }

    fn emitTwo(self : *Self , bt : ins.OpCode , bt2 : ins.OpCode) !void{
        try self.emitBt(bt);
        try self.emitBt(bt2);
    }

    fn emitTwoRaw(self : *Self , bt : u8 , bt2 : u8) !void{
        try self.emitBtRaw(bt);
        try self.emitBtRaw(bt2);
    }

    fn emitConst(self : *Self , val : PValue) !void{
        try self.emitBt(.Op_Const);
        try self.emitBtRaw(try self.makeConst(val));
    }

    fn getRule(t : lexer.TokenType) ParseRule{
        return rules.get(t);
    }

    fn makeConst(self : *Self , val : PValue ) !u8{
        const con : u8 = try self.inst.addConst(val);
        return con;
    }

    fn rNumber(self : *Self , _ : bool) !void{
        const stru8 = try utils.u32tou8(self.parser.previous.lexeme , self.al);
        const rawNum : f64 = try std.fmt.parseFloat(f64,stru8);
        try self.emitConst(PValue.makeNumber(rawNum));
        self.al.free(stru8);
    }

    fn rGroup(self : *Self , _ : bool) !void {
        try self.parseExpression();
        self.eat(.Rparen, "Expected ')' after group expression");
    }

    fn rString(self : *Self , _ : bool) !void{
        const s : *PObj.OString = try PObj.OString.copy(self.parser.vm, self.parser.previous.lexeme);
        try self.emitConst(s.obj.asValue());
    }

    fn rUnary(self : *Self , _ : bool) !void{
        const oprt = self.parser.previous.toktype;
        try self.parsePrec(.P_Unary);

        switch (oprt) {
            .Minus => { try self.emitBt(.Op_Neg); },
            .Bang => { try self.emitBt(.Op_Not); },
            else => { return; }
        }

        return;
    }

    fn rBinary(self : *Self , _ : bool) !void{
        const oprt : lexer.TokenType = self.parser.previous.toktype;
        const rule = Self.getRule(oprt);
        
        try self.parsePrec(@intToEnum(Precedence, @enumToInt(rule.prec) + 1));

        switch (oprt) {
            .Plus => try self.emitBt(.Op_Add),
            .Minus => try self.emitBt(.Op_Sub),
            .Astr => try self.emitBt(.Op_Mul),
            .Slash => try self.emitBt(.Op_Div),
            .NotEqual => try self.emitBt(.Op_Neq),
            .EqEq => try self.emitBt(.Op_Eq),
            .Gt => try self.emitBt(.Op_Gt),
            .Gte => try self.emitBt(.Op_Gte),
            .Lt => try self.emitBt(.Op_Lt),
            .Lte => try self.emitBt(.Op_Lte),
            else => {
                return;
            }
        }

        return;


    }
    
    fn rLiteral(self : *Self , _ : bool) !void {
        switch (self.parser.previous.toktype) {
            .False => { try self.emitBt(.Op_False);},
            .True => { try self.emitBt(.Op_True); },
            .Nil => { try self.emitBt(.Op_Nil); },
            else => { return; }
        }

        return;
    }

   
    fn parseExpression(self : *Self) !void{
        try self.parsePrec(.P_Assignment);
    }
    fn parsePrec(self : *Self , prec : Precedence) !void{
        self.parser.advance();

        const prefRule = Self.rules.get(self.parser.previous.toktype).prefix orelse {
            self.parser.err("Expected to get expression");
            return;
        };

        const canAssign = @enumToInt(prec) <= @enumToInt(Precedence.P_Assignment);

        try prefRule(self , canAssign);

        const precInt = @enumToInt(prec);
        while (precInt <= @enumToInt(Self.getRule(self.parser.current.toktype).prec)) {
            self.parser.advance();
            const infixRule = Self.getRule(self.parser.previous.toktype).infix orelse {
                self.parser.err("no infix rule found");
                return;
            };

            try infixRule(self , canAssign);
        }

        //if (canAssign and self) {}
    }

   
    pub fn eat(self : *Self , t : lexer.TokenType , msg : []const u8) void{
        if (self.parser.current.toktype == t) {
            self.parser.advance();
            return;
        }

        self.parser.errCur(msg);
    }

    pub fn curIns(self : *Self) *ins.Instruction{
        return self.inst;
    }
        
    fn endCompiler(self : *Self) !void {
        //try self.emitBt(.Nil);
        try self.emitBt(.Op_Return);
        if (!self.parser.hadErr) {
            self.inst.disasm("<script>");
        }
    }

    pub fn compile(self : *Self , source : []u32 , inst : *ins.Instruction) !bool{
        self.parser.init(source, self.al);
        self.inst = inst;
        
        self.parser.advance();

        //while (self.parser.previous.toktype != .Eof) {
        //    self.parser.advance();
        //}
        try self.parseExpression();

        try self.endCompiler();

        return !self.parser.hadErr;
    }


};

pub const Parser = struct {
    vm : *Vm,
    current : lexer.Token,
    previous : lexer.Token,
    lexer : lexer.Lexer,
    hadErr : bool,
    panicMode : bool,
    al : std.mem.Allocator,
    
    const Self = @This();

    pub fn new() Parser {
        return Parser{
            .current = lexer.Token.dummy(),
            .previous = lexer.Token.dummy(),
            .lexer = lexer.Lexer.dummy(),
            .hadErr = false,
            .panicMode = false,
            .al = undefined,
            .vm = undefined,
        };
    }

    pub fn init(self : *Self , source : []u32 , al : std.mem.Allocator) void{
        self.hadErr = false;
        self.panicMode = false;
        self.lexer = lexer.Lexer.new(source);
        self.current = undefined;
        self.previous = undefined;
        self.al = al;

            
    }

    fn errorAt(self : *Self, token : *lexer.Token , msg : []const u8) void {
        if (self.panicMode) { return; }
        std.debug.print("[L {d}] Error", .{token.line});
        if (token.toktype == .Eof) {
            std.debug.print(" at end" , .{});
        } else if (token.toktype == .Err){

        } else {
            std.debug.print(" at ''" , .{});
            utils.printu32(token.lexeme);
            std.debug.print("' ", .{});
        }

        std.debug.print(": {s}\n" , .{msg});
        self.hadErr = true;
    }

    fn errCur(self : *Self , msg : []const u8) void{
        self.errorAt(&self.current, msg);
    }

    fn err(self : *Self , msg : []const u8) void{
        self.errorAt(&self.previous , msg);
    }

    fn advance(self : *Self) void {
       self.previous = self.current;
       while (true) {
            self.current = self.lexer.getToken();
            if (self.current.toktype != .Err) {
                break;
            }
            const lxm = utils.u32tou8(self.current.lexeme, self.al) catch return;
            self.errCur(lxm);
            self.al.free(lxm);

       }
    }
    
};


