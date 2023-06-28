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
const Gc = @import("gc.zig").Gc;
const flags = @import("flags.zig");

const DEBUG = true;

pub const Local = struct {
    name : lexer.Token,
    depth : i32,
    isCaptured : bool,
};

pub const UpValue = struct {
    index : u8,
    isLocal : bool,
};

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

pub const FnType = enum(u8) {
    Ft_FUNC,
    Ft_SCRIPT, 
};

pub const Compiler = struct {
    parser : *Parser,
    gc : *Gc,
    localCount : u32,
    scopeDepth : u32,
    locals : [std.math.maxInt(u8)]Local,
    function : *PObj.OFunction,
    ftype : FnType,
    enclosing : ?*Compiler,
    upvalues : [std.math.maxInt(u8)]UpValue,


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
            .infix = Self.rCall,
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
        .PowAstr = ParseRule{
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
        .Identifer = ParseRule{
            .prefix = Self.rVariable,
        },
        .String = ParseRule{
           .prefix = Self.rString, 
        },
        .Number = ParseRule{
            .prefix = Self.rNumber,
            .prec = .P_None,
        },
        .And = ParseRule{
            .infix = Self.rAnd,
            .prec = .P_And,
        },

        .Or = ParseRule{
            .infix = Self.rOr,
            .prec = .P_Or,
        },
        .Do = ParseRule{},
        .If = ParseRule{},
        .Else = ParseRule{},
        .True = ParseRule{.prefix = Self.rLiteral},
        .False = ParseRule{.prefix = Self.rLiteral},
        .Nil = ParseRule{.prefix = Self.rLiteral},
        //.Show = ParseRule{},
        .Return = ParseRule{},
        .Let = ParseRule{},
        .PWhile = ParseRule{},
        .Err = ParseRule{},
        .Eof = ParseRule{},
        
    });
    
    pub fn init(self : *Self , gc : *Gc) void{
        self.parser  = Parser.new();
        self.parser.gc = gc;
    }
    
    pub fn newEnclosed(gc : *Gc , enclosing : ?*Self ,ftype : FnType) !Self{
        
        var function : *PObj.OFunction = try gc.newObj(.Ot_Function, PObj.OFunction);

        function.init(gc);

        var self = Self{
            .gc = gc,
            .enclosing = enclosing,
            .scopeDepth = 0,
            .function = function,
            .ftype = ftype,
            .localCount = 0,
            .upvalues = undefined,
            
            .parser = enclosing.?.parser,
            .locals = [_]Local{Local{ .name = lexer.Token.dummy(), .depth = 0 , .isCaptured = false }} ** std.math.maxInt(u8),
        };

        if (ftype != .Ft_SCRIPT) {
            self.function.name = try gc.copyString(enclosing.?.parser.previous.lexeme , enclosing.?.parser.previous.length);
        }

        var local = &self.locals[0];
        self.localCount = 1;
        local.depth = 0;
        local.name.lexeme = &[_]u32{};
        local.name.length = 0;
        local.isCaptured = false;

       gc.compiler = &self; 
        
        return self;

    }

    pub fn new(source : []u32 , gc : *Gc , ftype : FnType) !*Compiler{
        const c = try gc.getAlc().create(Compiler);
       
        c.* = .{
            .parser = try Parser.new(source, gc),
            .gc = gc,
            .scopeDepth = 0,
            .localCount = 0,
            .enclosing = null,
            .function = undefined,
            .upvalues = undefined,
            .ftype = ftype,
            .locals = [_]Local{Local{ .name = lexer.Token.dummy(), .depth = 0 , .isCaptured = false }} ** std.math.maxInt(u8),

        };
        
        var function : *PObj.OFunction = try gc.newObj(.Ot_Function,
                                                    PObj.OFunction);
        function.init(gc);

        c.*.function = function;
        c.*.gc.compiler = c;
        //var l = c.*.locals[0];
        //_ = l;

        
        //var local = &c.locals[0];
        //c.localCount += 1;
        //local.depth = 0;
        //local.name.lexeme = &[_]u32{};
        //local.name.length = 0;
        //local.captured = false;
        //

        c.*.locals[c.localCount].depth = 0;
        c.*.localCount+=1;


        return c;
    }

    fn beginScope(self : *Self) void {
        self.scopeDepth+=1;
    }

    fn endScope(self : *Self) void {
        self.scopeDepth-=1;

        while (self.localCount > 0 and 
            self.locals[self.localCount - 1].depth > self.scopeDepth) {
            
            if (self.locals[self.localCount-1].isCaptured) {


                self.emitBt(.Op_ClsUp) catch return;
            } 
            else {
                
                self.emitBt(.Op_Pop) catch return;
            }

            self.localCount -= 1;
        }
    }

    fn emitBtRaw(self : *Self , bt : u8) !void{
        try self.curIns().write_raw(bt, ins.InstPos.line(self.parser.previous.line));
    }

    fn emitBt(self : *Self , bt : ins.OpCode) !void{
        try self.curIns().write(bt, ins.InstPos.line(self.parser.previous.line));
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

    fn emitReturn(self : *Self) !void {
        try self.emitTwo(.Op_Nil, .Op_Return);
    }

    fn getRule(t : lexer.TokenType) ParseRule{
        return rules.get(t);
    }

    fn makeConst(self : *Self , val : PValue ) !u8{
        const con : u8 = try self.curIns().addConst(val);
        return con;
    }

    fn syncErrors(self : *Self) void {
        self.parser.panicMode = false;

        while (!self.parser.lexer.isEof()) {
            //if (self.parser.previous.toktype == .Semicolon) {
            //    return;
            //}

            switch (self.parser.current.toktype) {
               .Func , .Let , .If , .PWhile  , .Return => {
                    return;
               },

               else => {},
            }

            self.parser.advance();
        }
    }

    pub const UpvalueResolveError = error{
        UpNotFound,
    };
    
   

    fn resolveUpvalue(self : *Self , name : *const lexer.Token) UpvalueResolveError!i32{
        const enc = self.enclosing orelse return UpvalueResolveError.UpNotFound;
        
        const local = enc.resolveLocal(name);
        if (local) |l| {
            const ind : u8 = @truncate(@as(u64 , @intCast(l)));
            enc.locals[ind].isCaptured = true;
            return self.addUpvalue(ind, true);
        } else |_| {}

        const upv = enc.resolveUpvalue(name);
        if (upv) |u| {
            
            return self.addUpvalue(@truncate(@as(u64 , @intCast(u))), false);
        } else |_| {}

        return UpvalueResolveError.UpNotFound;
        
    }
    fn addUpvalue(self : *Self , index: u8 , isLocal : bool ) i32 {
        const upc = self.function.upvCount;
        //std.debug.print("UP_INDEX -> {d}\nUPC{d}\n" , .{index , upc});
        
        var i : u32 = 0;

        while (i < upc) : (i += 1) {
            const upv = self.upvalues[i];
            if (upv.index == index and upv.isLocal == isLocal) {
                return @intCast(i);
            }
        }

        if (upc == std.math.maxInt(u8)) {
            self.parser.err("too many closure variables");
            return 0;
        }

        self.upvalues[upc].index = index;
        self.upvalues[upc].isLocal = isLocal;
        self.function.upvCount += 1;
        return @intCast(upc);
    }


    fn addLocal(self : *Self , name : lexer.Token) void {
        
        if (self.localCount == std.math.maxInt(u8)) {
            self.parser.err("Too many local variables");
            return;
        }
        
        self.locals[self.localCount].name = name;
        self.locals[self.localCount].depth = -1;
        self.locals[self.localCount].isCaptured = false;
        self.localCount += 1;

    }

    pub const LocalResolveError = error {
       LocalNotFound, 
    };
    fn resolveLocal(self : *Self , name : *const lexer.Token) LocalResolveError!i32 {
        const locals = self.locals[0..self.localCount];
        var i  = @as(i64, self.localCount) - 1;

        while (i >= 0) : (i -= 1) {
            const local : Local = locals[@intCast(i)];

//            std.debug.print("FROM RESOLVE LOCAL ->> " , .{});
            if (idEqual(name , &local.name)) {
                if (local.depth == -1) {
                    self.parser.err("Can't read local variable in its own init");
                }

                return @intCast(i);
            }
        }

        return LocalResolveError.LocalNotFound;
    }

    fn readArgumentList(self : *Self) !u8 {
        var argc : u8 = 0;

        if (!self.check(.Rparen)) {
            while (true) {
                try self.parseExpression();

                if (argc == std.math.maxInt(u8)) {
                    self.parser.err("Can't have more than 255 arguments");
                }

                argc += 1;

                if (!self.match(.Comma)) {
                    break;
                }
            }
        }

        self.eat(.Rparen, "Expected ')' after arguments");
        return argc;
    }

    fn rCall(self : *Self , canAssign : bool) !void{
        _ = canAssign;
        const argc = try self.readArgumentList();
        try self.emitBt(.Op_Call);
        try self.emitBtRaw(argc);
    }

    fn rFuncDeclaration(self : *Self) !void{
        const global = self.parseVariable("Expected function name");
        self.markInit();
        try self.rFunc(.Ft_FUNC);
        try self.defineVar(global);
    }
    
   

    fn rFunc(self : *Self , ft : FnType) !void{
        _ = ft;

        var fcomp = try Self.newEnclosed(self.gc, self , .Ft_FUNC);

        fcomp.beginScope();

        fcomp.eat(.Lparen, "Expected (");
        if (!fcomp.check(.Rparen)) {
            while (true) {
                if (fcomp.function.arity == std.math.maxInt(u8)) {
                    fcomp.parser.errCur("cant have more than 255 arguments");
                    break;
                }

                fcomp.function.arity += 1;

                const con = fcomp.parseVariable("Expected param name");
                try fcomp.defineVar(con);

                if (!fcomp.match(.Comma)) {
                    break;
                }
            }
        }
        fcomp.eat(.Rparen, "Expected )");
        try fcomp.readToEnd();

        const f = try fcomp.endCompiler();
        try self.emitBt(.Op_Closure);
        try self.emitBtRaw(try self.makeConst(PValue.makeObj(f.parent())));

        var i : usize = 0;

        //std.debug.print("upc -> {d}\n" , .{f.upvCount});
        while (i < f.upvCount) : (i += 1) {
            const upv = &fcomp.upvalues[i];
            //std.debug.print("u{d} -> {any}\n" , .{i , upv});
            if (upv.isLocal) {
                try self.emitBtRaw(1);
            } else {
                try self.emitBtRaw(0);
            }
            //std.debug.print("-->UPVINDEX{d}\n" , .{upv.index});

            try self.emitBtRaw(upv.index);
        }

    }
    
    fn rVariable(self : *Self , canAssign : bool) !void {

        try self.namedVariable(self.parser.previous , canAssign);

    }

    fn rBlock(self : *Self) !void{
       while (!self.check(.Rbrace) and !self.check(.Eof)) {
            try self.rDeclaration();
       } 

       self.eat(.Rbrace, "Expected '}' after block statement");
    }

    fn namedVariable(self : *Self , name : lexer.Token , canAssign : bool) !void {

        var getOp : ins.OpCode = undefined;
        var setOp : ins.OpCode = undefined;

        //std.debug.print("SCOPE -> {d} {}\n" , .{self.scopeDepth , name});
        var arg = self.resolveLocal(&name) catch -1;

        if (arg != -1) {
            getOp = .Op_GetLocal;
            setOp = .Op_SetLocal;

        } else {
            
            arg = self.resolveUpvalue(&name) catch -1;
            if (arg != -1) {
                setOp = .Op_SetUp;
                getOp = .Op_GetUp;
            }else  {
                arg = self.rIdentConst(&name);
                getOp = .Op_GetGlob;
                setOp = .Op_SetGlob;
            }
        }
        
        if (canAssign and self.match(.Eq)) {

           try self.parseExpression();
           try self.emitBt(setOp);
           try self.emitBtRaw(@intCast(arg));
        } else {
            try self.emitBt(getOp);
            try self.emitBtRaw(@intCast(arg));
        }

        
    }



    fn rNumber(self : *Self , _ : bool) !void{
        const stru8 = try utils.u32tou8(self.parser.previous.lexeme , self.gc.getAlc());
        const rawNum : f64 = try std.fmt.parseFloat(f64,stru8);
        try self.emitConst(PValue.makeNumber(rawNum));
        self.gc.getAlc().free(stru8);
    }

    fn rGroup(self : *Self , _ : bool) !void {
        try self.parseExpression();
        self.eat(.Rparen, "Expected ')' after group expression");
    }

    fn rString(self : *Self , _ : bool) !void{
        const prevLen = self.parser.previous.lexeme.len;
        const s : *PObj.OString = try self.gc.copyString( 
                self.parser.previous.lexeme[1..prevLen - 1],
                self.parser.previous.length - 2,
                );

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
        
        try self.parsePrec(@enumFromInt(@intFromEnum(rule.prec) + 1));

        switch (oprt) {
            .Plus => try self.emitBt(.Op_Add),
            .Minus => try self.emitBt(.Op_Sub),
            .Astr => try self.emitBt(.Op_Mul),
            .PowAstr => try self.emitBt(.Op_Pow),
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

    fn rAnd(self : *Self , canAssign : bool) !void{
        _ = canAssign;

        const endJump = try self.emitJump(.Op_JumpIfFalse);
        try self.emitBt(.Op_Pop);
        try self.parsePrec(.P_And);
        self.patchJump(@intCast(endJump));

    }

    fn rOr(self : *Self , canAssign : bool) !void{
        _ = canAssign;
        const elseJump = try self.emitJump(.Op_JumpIfFalse);
        const endJump = try self.emitJump(.Op_Jump);

        self.patchJump(@intCast(elseJump));
        try self.emitBt(.Op_Pop);

        try self.parsePrec(.P_Or);
        self.patchJump(@intCast(endJump));
        
    }

    fn skipSemicolon(self : *Self) void {
        if (self.check(.Semicolon)) {
            self.eat(.Semicolon, "Ate Semicolon");
        }
    }

    fn rDeclaration(self : *Self) !void{

        if (self.match(.Let)) {
            try self.rLetDeclaration();
        } else if (self.match(.Func)) {
            try self.rFuncDeclaration();
        } else {
            try self.rStatement();
        }

        if (self.parser.panicMode) {
            self.syncErrors();
        }
    }

    fn rStatement(self : *Self) !void{
        if (self.match(.Return)){
            try self.rReturnStatement();
        } else if (self.match(.If)) {
            try self.rIfStatement();
        } else if (self.match(.PWhile)) {
            try self.rWhileStatement();
        } else if (self.match(.Lbrace)) {
            self.beginScope();
            try self.rBlock();
            self.endScope();
        }else {
            try self.rExprStatement();
        }

    }

    fn rReturnStatement(self : *Self) !void{
        
        if (self.ftype == .Ft_SCRIPT) {
            self.parser.err("Can't return from top-level code");
        }

        if (self.match(.Semicolon)) {
            try self.emitReturn();
        } else {
            try self.parseExpression();
            self.skipSemicolon();
            try self.emitBt(.Op_Return);
        }
    }

    fn rExprStatement(self : *Self) !void {
        self.parseExpression() catch {
            self.parser.err("failed to parse expression");
            return;
        };

        self.skipSemicolon();

        try self.emitBt(.Op_Pop);
    }

    

    fn rPrintStatement(self : *Self) !void {
        self.parseExpression() catch {
            self.parser.errCur("failed to parse expression");
            return;
        };


        
        self.skipSemicolon();

        try self.emitBt(.Op_Show);


        //self.inst.disasm("before print");
        //std.debug.print("next token -> {}, {}" , .{self.parser.previous , self.parser.current});

    }

    fn rLetDeclaration(self : *Self) !void {
        const global : u8 = self.parseVariable("Expected variable name");
        if (self.match(.Eq)) {
            try self.parseExpression();
        } else {
            try self.emitBt(.Op_Nil);
        }

        self.skipSemicolon();

        try self.defineVar(global);
    }

    fn defineVar(self : *Self , global : u8) !void{
        
        if (self.scopeDepth > 0) { 
            self.markInit();
            return;
        }

    
        try self.emitBt(.Op_DefGlob);
        try self.emitBtRaw(global);
    }
    
    fn parseVariable(self : *Self , msg : []const u8) u8{
        self.eat(.Identifer, msg);
 //       std.debug.print("==\n\nPREV->{any}  \nCUR->{} \n==\n" , .{self.parser.previous , self.parser.current});
        
        self.declareVariable();
        if (self.scopeDepth > 0 ) { return 0; }

        return self.rIdentConst(&self.parser.previous);
    }

    fn markInit(self : *Self) void {
        
        self.locals[self.localCount - 1].depth = @intCast(self.scopeDepth);
        
    }

    fn declareVariable(self : *Self) void {

        if (self.scopeDepth == 0) { return; }
        const name = self.parser.previous;
        
        const locals = self.locals[0..self.localCount];
        var i : i64 = @as(i64 , self.localCount) - 1;
        while (i >= 0) : (i -= 1) {
            const local : Local = locals[@intCast(i)];

            if (local.depth != -1 and local.depth < self.scopeDepth) {
                break;
            }

            if (idEqual(&name, &local.name)) {
                self.parser.err("Already a variable with this name in this scope");
            }
        }
        self.addLocal(name);
    }



    fn idEqual(a : *const lexer.Token , b : *const lexer.Token) bool {

        if (a.toktype != b.toktype) { return false; }
        if (a.length != b.length) { return false; }
        for (a.lexeme, 0..) |value, i| {
            if (value != b.lexeme[i]) { return false; }
        }
        return true;
    }

    fn rIdentConst(self : *Self, name : *const lexer.Token) u8 {
        const strObj : *PObj.OString = self.gc.copyString(name.lexeme , name.length) catch return 0;
        const val = strObj.parent().asValue();
         
        return self.makeConst(val) catch return 0;
    }

    fn readIfBlock(self : *Self) !void {
        self.beginScope();
        while (!self.check(.End) and !self.check(.Else) and !self.check(.Eof)) {
            try self.rDeclaration();
        }
        self.endScope();
    }

    fn readToEnd(self : *Self) !void{
        while (!self.check(.End) and !self.check(.Eof)) {
            try self.rDeclaration();
        }

        self.eat(.End, "Expected `end`");
    }

    fn rIfStatement(self : *Self) anyerror!void{
        try self.parseExpression();
        self.eat(.Then, "Expected then after if expression");

        const thenJump = try self.emitJump(.Op_JumpIfFalse);
        try self.emitBt(.Op_Pop);

        try self.readIfBlock();

        const elseJump = try self.emitJump(.Op_Jump);

        self.patchJump(@intCast(thenJump));
        try self.emitBt(.Op_Pop);

        if (self.match(.Else)) { 
            self.beginScope();
            try self.readToEnd();
            self.endScope();
        } else {
            self.eat(.End, "Expected `end` after if block without else");
        }

        self.patchJump(@intCast(elseJump));
        
    }

    fn rWhileStatement(self : *Self) !void{
        const loopStart = self.curIns().code.items.len;

        try self.parseExpression();
        self.eat(.Do, "Expected do after while expression");
        const exitJump = try self.emitJump(.Op_JumpIfFalse);
        try self.emitBt(.Op_Pop);
        self.beginScope();
        try self.readToEnd();
        self.endScope();
        try self.emitLoop(loopStart);
        self.patchJump(@intCast(exitJump));
        try self.emitBt(.Op_Pop);


    }

    fn emitLoop(self : *Self , loopStart : usize) !void{
        try self.emitBt(.Op_Loop);
        const offset = self.curIns().code.items.len - loopStart + 2;
        if (offset > std.math.maxInt(u16)) {
            self.parser.err("loop body too large");
        }

        const offu8 = utils.u16tou8(@intCast(offset));
        try self.emitTwoRaw(offu8[0], offu8[1]);
    }


    fn emitJump(self : *Self , instruction : ins.OpCode) !u32 {
        try self.emitBt(instruction);
        try self.emitBtRaw(0xff);
        try self.emitBtRaw(0xff);
        return @intCast(self.curIns().code.items.len - 2);
    }

    fn patchJump(self : *Self , offset : usize) void{
        const jump = self.curIns().code.items.len - offset - 2;

        if (jump > std.math.maxInt(u16)) {
            self.parser.err("Too much code to jump over");
        }

        const jmp : u16 = @intCast(jump);

        self.curIns().code.items[offset] = @intCast((jmp >> 8) & 0xff);
        self.curIns().code.items[offset + 1] = @intCast(jmp & 0xff);


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

        const canAssign = @intFromEnum(prec) <= @intFromEnum(Precedence.P_Assignment);

        try prefRule(self , canAssign);

        const precInt = @intFromEnum(prec);
        while (precInt <= @intFromEnum(Self.getRule(self.parser.current.toktype).prec)) {
            self.parser.advance();
            const infixRule = Self.getRule(self.parser.previous.toktype).infix orelse {
                self.parser.err("no infix rule found");
                return;
            };

            try infixRule(self , canAssign);
        }

        if (canAssign and self.match(.Eq)) {
            self.parser.err("Invalid assignment target");
        }
    }

   
    /// Eat the `t` TokenType from `current` token.  
    /// Otherwise raise an error with `msg`  as text
    pub fn eat(self : *Self , t : lexer.TokenType , msg : []const u8) void{
        if (self.parser.current.toktype == t) {
            self.parser.advance();
            return;
        }

        self.parser.errCur(msg);
    }

    /// Check if the `current` token is of type `t`
    pub fn check(self : *Self , t : lexer.TokenType) bool {
        return self.parser.current.toktype == t;
    }

    /// Check if current token is of type `t`, if yes  
    /// then advance the parser.
    pub fn match(self : *Self , t : lexer.TokenType) bool{
        if (!self.check(t)) { return false; }
        self.parser.advance();
        return true;
        
    }

    pub fn curIns(self : *Self) *ins.Instruction{
        return &self.function.ins;
    }
        
    fn endCompiler(self : *Self) !*PObj.OFunction {
        try self.emitBt(.Op_Nil);
        try self.emitBt(.Op_Return);
        const f = self.function;
        if (flags.DEBUG and flags.DEBUG_OPCODE) {
            const fname = try utils.u32tou8(self.function.getName() , 
                                            self.gc.getAlc());
            self.curIns().disasm(fname);
            self.gc.getAlc().free(fname);
            
        }

        return f;
    }

    pub fn compile(self : *Self , source : []u32) !?*PObj.OFunction{
        self.parser.init(source, self.gc);
        //self.inst = inst;
        
        self.parser.advance();

        while (!self.match(.Eof)) {
            try self.rDeclaration();
        }

        const f = try self.endCompiler();

        if (self.parser.hadErr) { return null; } else { return f; }
    }

    pub fn free(self : *Self , al : std.mem.Allocator) void{
        self.parser.free(self.gc.getAlc());
        al.destroy(self);
        
    }


};

pub const Parser = struct {
    gc : *Gc,
    current : lexer.Token,
    previous : lexer.Token,
    lexer : lexer.Lexer,
    hadErr : bool,
    panicMode : bool,
    
    const Self = @This();

    pub fn new(source : []u32 , gc : *Gc) !*Parser {
        const p = try gc.getAlc().create(Parser);
        p.* = .{
            .current = lexer.Token.dummy(),
            .previous = lexer.Token.dummy(),
            .lexer = lexer.Lexer.new(source),
            .hadErr = false,
            .panicMode = false,
            .gc = gc,
        };
        
        return p;
    }

    pub fn init(self : *Self , source : []u32 , gc : *Gc) void{
        
        if (flags.DEBUG and flags.DEBUG_LEXER) {
            var lx = lexer.Lexer.new(source);
            while (!lx.isEof()) {
                const tokStr =  lx.getToken().toString(self.gc.getAlc()) catch continue;
                std.debug.print("{s}\n", .{tokStr} );
                self.gc.getAlc().free(tokStr);
            }
            
        }

        self.hadErr = false;
        self.panicMode = false;
        self.lexer = lexer.Lexer.new(source);
        self.current = undefined;
        self.previous = undefined;
        self.gc = gc;

            
    }

    pub fn free(self : *Self , al : std.mem.Allocator) void{
        al.destroy(self);
    
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
            const lxm = utils.u32tou8(self.current.lexeme, self.gc.getAlc()) catch return;
            self.errCur(lxm);
            self.gc.getAlc().free(lxm);

       }
    }
    
};


