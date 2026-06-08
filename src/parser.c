/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "include/parser.h"
#include "external/stb/stb_ds.h"
#include "gen/diagon.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/diagonctx.h"
#include "include/gc.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/utils.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_UNICODE_CP 0xFFFD
// Create New Let Statement;
// Returns PStmt with type `STMT_LET`
static PStmt *rLet(Parser *p);

// Create New Statement
static PStmt *rStmt(Parser *p);

// Read assignment expression;
// Returns expression with type `EXPR_ASSIGN`
static PExpr *rAssignment(Parser *p);
static PExpr *rEquality(Parser *p);
static PExpr *rComparison(Parser *p);
static PExpr *rTerm(Parser *p);
static PExpr *rFactor(Parser *p);
static PExpr *rUnary(Parser *p);
static PExpr *rExponent(Parser *p);
static PExpr *rCall(Parser *p);
static PExpr *rPrimary(Parser *p);
static PExpr *rExpression(Parser *p);

// Check if parser has reached end
static bool atEnd(const Parser *p);

Parser *NewParser(Pgc *gc, Lexer *lexer) {
    Parser *parser = PCreate(Parser);
    if (parser == NULL) {
        return NULL;
    }
    // ERROR HANDLE
    parser->lx = lexer;
    parser->tokens = lexer->tokens;
    parser->gc = gc;
    parser->pos = 0;
    parser->stmts = NULL;
    parser->hasError = false;
    return parser;
}
void FreeParser(Parser *parser) {
    if (parser == NULL) {
        return;
    }
    arrfree(parser->stmts);
    PFree(parser);
}

PStmt **ParseParser(Parser *parser) {
    while (!atEnd(parser)) {
        arrput(parser->stmts, rLet(parser));
    }

    return parser->stmts;
}

static Token *peek(const Parser *p) { return p->tokens[p->pos]; }

static bool atEnd(const Parser *p) { return peek(p)->type == T_EOF; }

static bool check(const Parser *p, PTokenType t) {

    if (atEnd(p)) {
        return false;
    }
    return peek(p)->type == t;
}

static Token *previous(const Parser *p) { return p->tokens[p->pos - 1]; }

static Token *advance(Parser *p) {
    if (!atEnd(p)) {
        p->pos++;
    }

    return previous(p);
}

static bool matchMany(Parser *p, PTokenType *types, int count) {
    for (int i = 0; i < count; i++) {
        if (check(p, types[i])) {
            advance(p);
            return true;
        }
    }

    return false;
}

static bool matchOne(Parser *p, PTokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }

    return false;
}

static inline void error(Parser *p, Token *tok, PanDiagCode code, ...) {
    p->hasError = true;
    va_list args;
    va_start(args, code);
    ReportDiagV(&p->errCtx, tok, code, args);
    va_end(args);
}

static Token *eat(Parser *p, PTokenType t, PanDiagCode code) {
    if (check(p, t)) {
        return advance(p);
    }

    error(p, peek(p), code);
    return NULL;
}

static PExpr *rExpression(Parser *p) { return rAssignment(p); }

static PExpr *rAnd(Parser *p) {
    PExpr *expr = rEquality(p);
    while (matchOne(p, T_AND)) {
        Token *op = previous(p);
        PExpr *right = rEquality(p);
        expr = NewLogical(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_LOGICAL_EXPR);
            return NULL;
        }
    }
    return expr;
}

static PExpr *rOr(Parser *p) {
    PExpr *expr = rAnd(p);
    while (matchOne(p, T_OR)) {
        Token *op = previous(p);
        PExpr *right = rAnd(p);
        expr = NewLogical(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_LOGICAL_EXPR);
            return NULL;
        }
    }
    return expr;
}

static PExpr *rAssignment(Parser *p) {
    PExpr *expr = rOr(p);
    if (matchOne(p, T_EQ)) {
        Token *op = previous(p);
        PExpr *value = rAssignment(p);

        if (expr->type == EXPR_VARIABLE || expr->type == EXPR_SUBSCRIPT) {
            PExpr *assignExpr = NewAssignment(p->gc, op, expr, value);
            if (assignExpr == NULL) {
                error(p, op, PARSER_IME_ASSIGN_EXPR);
                return NULL;
            }
            return assignExpr;
        }

        error(p, NULL, PARSER_INVALID_ASSIGN);
        return NULL;
    }

    return expr;
}

static PExpr *rEquality(Parser *p) {
    PExpr *expr = rComparison(p);
    while (matchMany(p, (PTokenType[]){T_BANG_EQ, T_EQEQ}, 2)) {
        Token *op = previous(p);
        PExpr *right = rComparison(p);

        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_BINARY_EXPR);
            return NULL;
        }
    }

    return expr;
}

static PExpr *rComparison(Parser *p) {
    PExpr *expr = rTerm(p);
    while (matchMany(p, (PTokenType[]){T_GT, T_GTE, T_LT, T_LTE}, 4)) {
        Token *op = previous(p);
        PExpr *right = rTerm(p);
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_BINARY_EXPR);
            return NULL;
        }
    }

    return expr;
}

static PExpr *rTerm(Parser *p) {
    PExpr *expr = rFactor(p);
    while (matchMany(p, (PTokenType[]){T_MINUS, T_PLUS}, 2)) {
        Token *op = previous(p);
        PExpr *right = rFactor(p);
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_BINARY_EXPR);
            return NULL;
        }
    }

    return expr;
}

static PExpr *rFactor(Parser *p) {
    PExpr *expr = rUnary(p);
    while (matchMany(p, (PTokenType[]){T_SLASH, T_ASTR, T_MOD}, 3)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p);
        if (right == NULL) {
            error(p, NULL, PARSER_INVALID_MULDIV_RIGHT);
        }
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_BINARY_EXPR);
            return NULL;
        }
    }
    return expr;
}

static PExpr *rUnary(Parser *p) {
    if (matchMany(p, (PTokenType[]){T_BANG, T_MINUS}, 2)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p);
        PExpr *unaryExpr = NewUnary(p->gc, op, right);
        if (unaryExpr == NULL) {
            error(p, op, PARSER_IME_UNARY_EXPR);
            return NULL;
        }
        return unaryExpr;
    }

    return rExponent(p);
}

static PExpr *rExponent(Parser *p) {
    PExpr *expr = rCall(p);
    if (matchOne(p, T_EXPONENT)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p); // <-- This might seem confusing
        // why are we going up?
        // but without it `a ** -b()` throws error, cause the unary getting
        // skipped as the parser directly going to the call
        if (right == NULL) {
            error(p, NULL, PARSER_INVALID_EXPO_RIGHT);
        }
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            error(p, op, PARSER_IME_BINARY_EXPR);
            return NULL;
        }
    }
    return expr;
}

static PExpr *finishCallExpr(Parser *p, PExpr *expr) {
    PExpr **args = NULL;
    u64 count = 0;
    if (!check(p, T_RIGHT_PAREN)) {
        do {
            if (count >= 255) {
                error(p, NULL, PARSER_CALL_TOOMANY_ARGS);
            }
            arrput(args, rExpression(p));
            count++;
        } while (matchOne(p, T_COMMA));
    }

    Token *rparen = eat(p, T_RIGHT_PAREN, PARSER_EXPECT_CALL_RPAREN);
    PExpr *callExpr = NewCallExpr(p->gc, rparen, expr, args, count);
    if (callExpr == NULL) {
        error(p, rparen, PARSER_IME_CALL_EXPR);
        return NULL;
    }
    return callExpr;
}

static PExpr *rSubscript(Parser *p, PExpr *expr) {
    Token *op = previous(p);
    PExpr *indexExpr = rExpression(p);
    if (indexExpr == NULL) {
        error(p, op, PARSER_INVALID_SUBEXPR_INDEX);
        return NULL;
    }
    eat(p, T_RS_BRACKET, PARSER_EXPECT_RBRACKET_SUBEXPR);
    PExpr *subExpr = NewSubscriptExpr(p->gc, op, expr, indexExpr);
    if (subExpr == NULL) {
        error(p, op, PARSER_IME_SUBSCRIPT_EXPR);
        return NULL;
    }

    return subExpr;
}

// Module get expression `module.child`
static PExpr *rModget(Parser *p, PExpr *expr) {
    Token *op = previous(p);
    Token *childTok = eat(p, T_IDENT, PARSER_EXPECT_MOD_CHILD);
    PExpr *modGetExpr = NewModgetExpr(p->gc, op, expr, childTok);
    if (modGetExpr == NULL) {
        error(
            p, op, PARSER_IME_MOD_CHILD

        );
        return NULL;
    }

    return modGetExpr;
}

static PExpr *rCall(Parser *p) {
    PExpr *expr = rPrimary(p);
    while (true) {
        if (matchOne(p, T_LEFT_PAREN)) {
            expr = finishCallExpr(p, expr);
        } else if (matchOne(p, T_LS_BRACKET)) {
            expr = rSubscript(p, expr);
        } else if (matchOne(p, T_DOT)) {
            expr = rModget(p, expr);
        } else {
            break;
        }
    }

    return expr;
}

static PExpr *rArrayExpr(Parser *p) {
    PExpr **items = NULL;

    while (!check(p, T_RS_BRACKET)) {
        arrput(items, rExpression(p));
        if (check(p, T_RS_BRACKET)) {
            break;
        }
        if (eat(p, T_COMMA, PARSER_EXPECT_COMMA_ARRAYITEM) == NULL) {
            break;
        }
    }
    Token *rbrace = eat(p, T_RS_BRACKET, PARSER_EXPECT_RBRACKET_ARRAY);
    u64 itemCount = (u64)arrlen(items);
    return NewArrayExpr(p->gc, rbrace, items, itemCount);
}

static PExpr *rMapExpr(Parser *p) {
    PExpr **etable = NULL;
    Token *lbrace = previous(p);
    while (!check(p, T_RIGHT_BRACE)) {
        arrput(etable, rExpression(p));
        eat(p, T_COLON, PARSER_EXPECT_COLON_MAPKEY);
        arrput(etable, rExpression(p));
        if (check(p, T_RIGHT_BRACE)) {
            break;
        }
        eat(p, T_COMMA, PARSER_EXPECT_COMMA_MAPPAIR);
    }
    eat(p, T_RIGHT_BRACE, PARSER_EXPECT_RBRACE_MAP);
    u64 itemCount = (u64)(arrlen(etable));
    return NewMapExpr(p->gc, lbrace, etable, itemCount);
}

static PExpr *rPrimary(Parser *p) {
    if (matchOne(p, T_TRUE)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, previous(p), EXP_LIT_BOOL);
        if (e == NULL) {
            error(p, op, PARSER_IME_BOOL_EXPR);
            return NULL;
        }
        e->exp.ELiteral.value.bvalue = true;
        return e;
    }

    if (matchOne(p, T_FALSE)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, op, EXP_LIT_BOOL);
        if (e == NULL) {
            error(p, op, PARSER_IME_BOOL_EXPR);
            return NULL;
        }
        e->exp.ELiteral.value.bvalue = false;
        return e;
    }

    if (matchOne(p, T_NIL)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, op, EXP_LIT_NIL);
        if (e == NULL) {
            error(p, op, PARSER_IME_NIL_EXPR);
        }

        return e;
    }

    if (matchOne(p, T_NUM)) {
        Token *opTok = previous(p);

        bool ok = true;
        double value = NumberFromStr(opTok->lexeme, opTok->len, &ok);
        if (!ok) {
            error(p, opTok, PARSER_MALFORMED_NUMBER);
            return NULL;
        }

        PExpr *e = NewLiteral(p->gc, opTok, EXP_LIT_NUM);
        if (e == NULL) {
            error(p, opTok, PARSER_IME_NUM_EXPR);
            return NULL;
        }
        e->exp.ELiteral.value.nvalue = value;
        return e;
    }

    if (matchOne(p, T_STR)) {
        Token *opTok = previous(p);
        PExpr *e = NewLiteral(p->gc, opTok, EXP_LIT_STR);
        if (e == NULL) {
            error(p, opTok, PARSER_IME_STR_EXPR);
            return NULL;
        }

        return e;
    }

    if (matchOne(p, T_IDENT)) {
        Token *op = previous(p);
        PExpr *e = NewVarExpr(p->gc, op);
        if (e == NULL) {
            error(p, op, PARSER_IME_IDENT_EXPR);
            return NULL;
        }
        return e;
    }

    if (matchOne(p, T_LEFT_PAREN)) {
        Token *op = previous(p);
        PExpr *e = rExpression(p);
        eat(p, T_RIGHT_PAREN, PARSER_EXPECT_RPAREN_GROUP);
        PExpr *grpExpr = NewGrouping(p->gc, op, e);
        if (grpExpr == NULL) {
            error(p, op, PARSER_IME_GROUP_EXPR);
            return NULL;
        }

        return grpExpr;
    }

    if (matchOne(p, T_LS_BRACKET)) {
        Token *op = previous(p);
        PExpr *e = rArrayExpr(p);
        if (e == NULL) {
            error(p, op, PARSER_IME_ARRAY_EXPR);
            return NULL;
        }

        return e;
    }

    if (matchOne(p, T_LEFT_BRACE)) {
        Token *op = previous(p);
        PExpr *e = rMapExpr(p);
        if (e == NULL) {
            error(p, op, PARSER_IME_MAP_EXPR);
            return NULL;
        }

        return e;
    }

    error(p, previous(p), PARSER_EXPECT_EXPR);
    return NULL;
}

static void syncParser(Parser *p) {
    advance(p);
    while (!atEnd(p)) {
        if (previous(p)->type == T_SEMICOLON) {
            return;
        }

        switch (peek(p)->type) {
            case T_FUNC:
            case T_LET:
            case T_WHILE:
            case T_IF:
            case T_RETURN:
            case T_IMPORT: return;
            default: advance(p); break;
        }
    }
}

static PStmt *rExprStmt(Parser *p) {
    PExpr *value = rExpression(p);
    Token *op = previous(p);
    PStmt *exprStmt = NewExprStmt(p->gc, op, value);

    if (exprStmt == NULL) {
        error(p, op, PARSER_IME_EXPR_STMT);
        return NULL;
    }

    return exprStmt;
}

static PStmt *rDebugStmt(Parser *p) {
    PExpr *value = rExpression(p);
    Token *op = previous(p);
    PStmt *debugStmt = NewDebugStmt(p->gc, op, value);
    if (debugStmt == NULL) {
        error(p, op, PARSER_IME_DEBUG_STMT);
        return NULL;
    }

    return debugStmt;
}

static PStmt *rLetStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, PARSER_EXPECT_LET_IDENT);
    eat(p, T_EQ, PARSER_EXPECT_EQ_LET_IDENT);
    PExpr *value = rExpression(p);
    PStmt *letStmt = NewLetStmt(p->gc, name, value);
    if (letStmt == NULL) {
        error(p, name, PARSER_IME_LET_STMT);
        return NULL;
    }

    return letStmt;
}

static PStmt *rBlockStmt(Parser *p) {
    Token *curTok = previous(p);
    PStmt **stmtList = NULL;
    while (!check(p, T_RIGHT_BRACE) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    eat(p, T_RIGHT_BRACE, PARSER_EXPECT_RBRACE_BLOCK);
    PStmt *block = NewBlockStmt(p->gc, curTok, stmtList);
    if (block == NULL) {
        error(p, curTok, PARSER_IME_BLOCK_STMT);
        return NULL;
    }
    return block;
}

static PStmt *rToEndBlockStmt(Parser *p) {
    PStmt **stmtList = NULL;
    while (!check(p, T_END) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    eat(p, T_END, PARSER_EXPECT_END_BLOCK);
    Token *endTok = previous(p);
    PStmt *block = NewBlockStmt(p->gc, previous(p), stmtList);
    if (block == NULL) {
        error(p, endTok, PARSER_IME_BLOCK_STMT);
        return NULL;
    }
    return block;
}

static PStmt *rToEndOrElseBlockStmt(Parser *p, bool *hasElse) {
    PStmt **stmtList = NULL;

    while ((!check(p, T_END) && !check(p, T_ELSE)) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    if (check(p, T_ELSE)) {
        eat(p, T_ELSE, PARSER_EXPECT_NOOP_ELSE_BLOCK);
        *hasElse = true;
    } else if (check(p, T_END)) {
        eat(p, T_END, PARSER_EXPECT_NOOP_END_BLOCK);
        *hasElse = false;
    }

    Token *op = previous(p);

    PStmt *block = NewBlockStmt(p->gc, op, stmtList);
    if (block == NULL) {
        error(p, op, PARSER_IME_BLOCK_STMT);
        return NULL;
    }
    return block;
}
static PStmt *rIfStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_THEN, PARSER_EXPECT_THEN_IFCOND);
    bool hasElse = false;
    PStmt *thenBranch = rToEndOrElseBlockStmt(p, &hasElse);
    PStmt *elseBranch = NULL;
    if (hasElse) {
        if (check(p, T_IF)) {
            advance(p);
            elseBranch = rIfStmt(p);
        } else {
            elseBranch = rToEndBlockStmt(p);
        }
    }

    PStmt *ifStmt = NewIfStmt(p->gc, op, cond, thenBranch, elseBranch);
    if (ifStmt == NULL) {
        error(p, op, PARSER_IME_IF_STMT);
        return NULL;
    }

    return ifStmt;
}

static PStmt *rWhileStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_DO, PARSER_EXPECT_DO_WHILECOND);
    PStmt *body = rToEndBlockStmt(p);
    PStmt *whileStmt = NewWhileStmt(p->gc, op, cond, body);

    if (whileStmt == NULL) {
        error(p, op, PARSER_IME_WHILE_STMT);
        return NULL;
    }

    return whileStmt;
}

static PStmt *rReturnStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *value = NULL;

    if (!check(p, T_SEMICOLON)) {
        value = rExpression(p);
    }

    if (check(p, T_SEMICOLON)) {

        eat(p, T_SEMICOLON, PARSER_EXPECT_SEMICOLON_NILRETURN);
    }

    PStmt *retStmt = NewReturnStmt(p->gc, op, value);
    if (retStmt == NULL) {
        error(p, op, PARSER_IME_RETURN_STMT);
        return NULL;
    }

    return retStmt;
}

static PStmt *rBreakStmt(Parser *p) {
    Token *op = previous(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, PARSER_EXPECT_SEMICOLON);
    }

    PStmt *breakStmt = NewBreakStmt(p->gc, op);

    if (breakStmt == NULL) {
        error(p, op, PARSER_IME_BREAK_STMT);
        return NULL;
    }

    return breakStmt;
}

static PStmt *rContinueStmt(Parser *p) {
    Token *op = previous(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, PARSER_EXPECT_SEMICOLON);
    }

    PStmt *contStmt = NewContinueStmt(p->gc, op);
    if (contStmt == NULL) {
        error(p, op, PARSER_IME_CONTINUE_STMT);
        return NULL;
    }

    return contStmt;
}

static PStmt *rFuncStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, PARSER_EXPECT_FUNC_NAME);
    eat(p, T_LEFT_PAREN, PARSER_EXPECT_LPAREN_FUNC_NAME);
    Token **params = NULL;
    u64 paramCount = 0;
    if (!check(p, T_RIGHT_PAREN)) {
        do {
            Token *param = eat(p, T_IDENT, PARSER_EXPECT_FUNC_PARAM);
            arrput(params, param);
            paramCount++;
        } while (matchOne(p, T_COMMA));
    }
    eat(p, T_RIGHT_PAREN, PARSER_EXPECT_FUNC_RPAREN);
    PStmt *body = rToEndBlockStmt(p);

    PStmt *funcStmt = NewFuncStmt(p->gc, name, params, body, paramCount);
    if (funcStmt == NULL) {
        error(p, name, PARSER_IME_FUNC_STMT);
        return NULL;
    }

    return funcStmt;
}

static PStmt *rImportStmt(Parser *p) {
    Token *op = previous(p);
    Token *name = eat(p, T_IDENT, PARSER_EXPECT_IMPORT_NAME);
    PExpr *pathExpr = rExpression(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON,
            PARSER_EXPECT_SEMICOLON); // Error should never occur
    }

    PStmt *importStmt = NewImportStmt(p->gc, op, name, pathExpr);
    if (importStmt == NULL) {
        error(p, op, PARSER_IME_IMPORT_STMT);
        return NULL;
    }

    return importStmt;
}

static PStmt *rStmt(Parser *p) {
    if (matchOne(p, T_LEFT_BRACE)) {
        return rBlockStmt(p);
    } else if (matchOne(p, T_IF)) {
        return rIfStmt(p);
    } else if (matchOne(p, T_WHILE)) {
        return rWhileStmt(p);
    } else if (matchOne(p, T_RETURN)) {
        return rReturnStmt(p);
    } else if (matchOne(p, T_BREAK)) {
        return rBreakStmt(p);
    } else if (matchOne(p, T_FUNC)) {
        return rFuncStmt(p);
    } else if (matchOne(p, T_IMPORT)) {
        return rImportStmt(p);
    } else if (matchOne(p, T_DEBUG)) {
        return rDebugStmt(p);
    } else if (matchOne(p, T_CONTINUE)) {
        return rContinueStmt(p);
    }
    return rExprStmt(p);
}

static PStmt *rLet(Parser *p) {
    if (matchOne(p, T_LET)) {
        return rLetStmt(p);
    }

    PStmt *s = rStmt(p);
    if (p->hasError) {
        syncParser(p);
    }

    return s;
}
