#include "include/parser.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/lexer.h"
#include "include/strescape.h"
#include "include/token.h"
#include "include/utils.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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
    // ERROR HANDLE
    parser->lx = lexer;
    parser->tokens = lexer->tokens;
    parser->gc = gc;
    parser->pos = 0;
    parser->core = NULL;
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

static inline void error(Parser *p, Token *tok, char *msg) {
    p->hasError = true;
    CoreParserError(p->core, tok, msg, false);
}

static inline void fatalError(Parser *p, Token *tok, char *msg) {
    p->hasError = true;
    CoreParserError(p->core, tok, msg, true);
}

static Token *eat(Parser *p, PTokenType t, char *msg) {
    if (check(p, t)) {
        return advance(p);
    }

    error(p, peek(p), msg);
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create logical expression "
                "while reading 'and'"
            );
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create logical expression "
                "while reading 'or'"
            );
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
                fatalError(
                    p, op,
                    "Internal Memory Error : Failed to create assignment "
                    "expression"
                );
                return NULL;
            }
            return assignExpr;
        }

        error(p, NULL, "Invalid assignment");
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create binary expression "
                "when reading equality expression"
            );
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create binary expression "
                "when reading comparison expression"
            );
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create binary expression "
                "when reading Add/Sub expression"
            );
            return NULL;
        }
    }

    return expr;
}

static PExpr *rFactor(Parser *p) {
    PExpr *expr = rUnary(p);
    while (matchMany(p, (PTokenType[]){T_SLASH, T_ASTR}, 2)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p);
        if (right == NULL) {
            error(p, NULL, "Invaid expression found in factor expression");
        }
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create binary expression "
                "when reading Mul/Div expression"
            );
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
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create unary expression"
            );
            return NULL;
        }
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
            error(p, NULL, "Invalid expression found in exponent expression");
        }
        expr = NewBinaryExpr(p->gc, expr, op, right);
        if (expr == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create binary expression "
                "when reading exponent expression"
            );
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
                error(p, NULL, "Can't have more than 255 arguments");
            }
            arrput(args, rExpression(p));
            count++;
        } while (matchOne(p, T_COMMA));
    }

    Token *rparen = eat(p, T_RIGHT_PAREN, "Expected ')' after arguments");
    PExpr *callExpr = NewCallExpr(p->gc, rparen, expr, args, count);
    if (callExpr == NULL) {
        fatalError(
            p, rparen,
            "Internal Memory Error : Failed to create call expression"
        );
        return NULL;
    }
    return callExpr;
}

static PExpr *rSubscript(Parser *p, PExpr *expr) {
    Token *op = previous(p);
    PExpr *indexExpr = rExpression(p);
    if (indexExpr == NULL) {
        error(p, op, "Invalid subscript index");
        return NULL;
    }
    eat(p, T_RS_BRACKET, "Expected ']' after subscript expression");
    PExpr *subExpr = NewSubscriptExpr(p->gc, op, expr, indexExpr);
    if (subExpr == NULL) {
        fatalError(
            p, op,
            "Internal Memory Error : Failed to create subscript expression"
        );
        return NULL;
    }

    return subExpr;
}

// Module get expression `module.child`
static PExpr *rModget(Parser *p, PExpr *expr) {
    Token *op = previous(p);
    Token *childTok = eat(p, T_IDENT, "Expected child token");
    PExpr *modGetExpr = NewModgetExpr(p->gc, op, expr, childTok);
    if (modGetExpr == NULL) {
        fatalError(
            p, op,
            "Internal Memory Error : Failed to create module child get "
            "expression"
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
        if (eat(p, T_COMMA, "Expected ',' comma after array item") == NULL) {
            break;
        }
    }
    Token *rbrace = eat(p, T_RS_BRACKET, "Expected ']' after array items");
    u64 itemCount = (u64)arrlen(items);
    return NewArrayExpr(p->gc, rbrace, items, itemCount);
}

static PExpr *rMapExpr(Parser *p) {
    PExpr **etable = NULL;
    Token *lbrace = previous(p);
    while (!check(p, T_RIGHT_BRACE)) {
        arrput(etable, rExpression(p));
        eat(p, T_COLON, "Expected ':' after map key");
        arrput(etable, rExpression(p));
        if (check(p, T_RIGHT_BRACE)) {
            break;
        }
        eat(p, T_COMMA, "Expected ',' after map pair");
    }
    eat(p, T_RIGHT_BRACE, "Expected '}' after map");
    u64 itemCount = (u64)(arrlen(etable));
    return NewMapExpr(p->gc, lbrace, etable, itemCount);
}

static char *readStringEscapes(Parser *p, Token *tok) {
    char *rawinput = tok->lexeme;
    u64 inlen = (u64)strlen(rawinput);
    u64 outlen = inlen * 4 + 1;
    char *output = PCalloc((u64)outlen, sizeof(char));
    if (output == NULL) {
        return NULL;
    }

    StrEscapeErr err = ProcessStringEscape(rawinput, inlen, output, outlen);
    switch (err) {
        case SESC_OK: break;
        case SESC_UNKNOWN_ESCAPE: {
            error(p, tok, "Unknown escape sequence found in string");
            break;
        }
        case SESC_INVALID_HEX_CHAR: {
            error(
                p, tok, "Invalid Hex character found in string escape sequence"
            );
            break;
        }

        case SESC_BUFFER_NOT_ENOUGH: {
            error(p, tok, "Internal Error: Failed to process string escape");
            break;
        }
        case SESC_INPUT_FINISHED_EARLY: {
            error(
                p, tok,
                "Incomplete escape sequence found at the end of the string"
            );
            break;
        }

        case SESC_NO_LOW_SURROGATE: {
            error(
                p, tok,
                "While reading \\uHHHH sequence another low surrogate \\uHHHH "
                "sequence was expected but not found"
            );
            break;
        }
        case SESC_LONE_LOW_SURROGATE: {
            error(
                p, tok,
                "While reading \\uHHHH sequence only a lone low surrogate was "
                "found"
            );
            break;
        }

        case SESC_INVALID_LOW_SURROGATE: {
            error(
                p, tok,
                "While reading \\uHHHH a invalid low surrogate was found after "
                "a high surrogate"
            );
            break;
        }
        case SESC_8_INVALID_CP: {
            error(p, tok, "Found invalid codepoint in \\uHHHHHHHH sequence");
            break;
        }
        case SESC_NULL_PTR: {
            error(
                p, tok,
                "Internal Error: Invalid Memory allocation for output in "
                "reading escape sequences"
            );
            break;
        }
    }
    return output;
}

static PExpr *rPrimary(Parser *p) {
    if (matchOne(p, T_TRUE)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, previous(p), EXP_LIT_BOOL);
        if (e == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create bool literal "
                "expression"
            );
            return NULL;
        }
        e->exp.ELiteral.value.bvalue = true;
        return e;
    }

    if (matchOne(p, T_FALSE)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, op, EXP_LIT_BOOL);
        if (e == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create bool literal "
                "expression"
            );
            return NULL;
        }
        e->exp.ELiteral.value.bvalue = false;
        return e;
    }

    if (matchOne(p, T_NIL)) {
        Token *op = previous(p);
        PExpr *e = NewLiteral(p->gc, op, EXP_LIT_NIL);
        if (e == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create nil literal "
                "expression"
            );
        }

        return e;
    }

    if (matchOne(p, T_NUM)) {
        Token *opTok = previous(p);

        bool ok = true;
        double value = NumberFromStr(opTok->lexeme, opTok->len, &ok);
        if (!ok) {
            error(p, opTok, "Invalid or malformed number found");
            return NULL;
        }

        PExpr *e = NewLiteral(p->gc, opTok, EXP_LIT_NUM);
        if (e == NULL) {
            fatalError(
                p, opTok,
                "Internal Memory Error : Failed to create number literal "
                "expression"
            );
            return NULL;
        }
        e->exp.ELiteral.value.nvalue = value;
        return e;
    }

    if (matchOne(p, T_STR)) {
        Token *opTok = previous(p);
        char *escapedStr = readStringEscapes(p, opTok);

        if (escapedStr == NULL) {
            fatalError(
                p, opTok,
                "Internal Memory Error : Failed to create string literal's "
                "characters"
            );
            return NULL;
        }

        PExpr *e = NewLiteral(p->gc, previous(p), EXP_LIT_STR);
        if (e == NULL) {
            fatalError(
                p, opTok,
                "Internal Memory Error : Failed to create string literal "
                "expression"
            );
            return NULL;
        }

        e->exp.ELiteral.value.svalue = escapedStr;

        return e;
    }

    if (matchOne(p, T_IDENT)) {
        Token *op = previous(p);
        PExpr *e = NewVarExpr(p->gc, op);
        if (e == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create identifier expression"
            );
            return NULL;
        }
        return e;
    }

    if (matchOne(p, T_LEFT_PAREN)) {
        Token *op = previous(p);
        PExpr *e = rExpression(p);
        eat(p, T_RIGHT_PAREN, "Expected ')'");
        PExpr *grpExpr = NewGrouping(p->gc, op, e);
        if (grpExpr == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create group expression"
            );
            return NULL;
        }

        return grpExpr;
    }

    if (matchOne(p, T_LS_BRACKET)) {
        Token *op = previous(p);
        PExpr *e = rArrayExpr(p);
        if (e == NULL) {
            fatalError(
                p, op,
                "Internal Memory Error : Failed to create array expression"
            );
            return NULL;
        }

        return e;
    }

    if (matchOne(p, T_LEFT_BRACE)) {
        Token *op = previous(p);
        PExpr *e = rMapExpr(p);
        if (e == NULL) {
            fatalError(
                p, op, "Internal Memory Error : Failed to create map expression"
            );
            return NULL;
        }

        return e;
    }

    error(p, previous(p), "Expected expression");
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
        fatalError(
            p, op,
            "Internal Memory Error : Failed to create expression statement"
        );
        return NULL;
    }

    return exprStmt;
}

static PStmt *rDebugStmt(Parser *p) {
    PExpr *value = rExpression(p);
    Token *op = previous(p);
    PStmt *debugStmt = NewDebugStmt(p->gc, op, value);
    if (debugStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create debug statement"
        );
        return NULL;
    }

    return debugStmt;
}

static PStmt *rLetStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, "Expected Identifier");
    eat(p, T_EQ, "Expected equal");
    PExpr *value = rExpression(p);
    PStmt *letStmt = NewLetStmt(p->gc, name, value);
    if (letStmt == NULL) {
        fatalError(
            p, name, "Internal Memory Error : Failed to create let statement"
        );
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

    eat(p, T_RIGHT_BRACE, "Expected '}' after block stmt");
    PStmt *block = NewBlockStmt(p->gc, curTok, stmtList);
    if (block == NULL) {
        fatalError(
            p, curTok,
            "Internal Memory Error : Failed to create block statement"
        );
        return NULL;
    }
    return block;
}

static PStmt *rToEndBlockStmt(Parser *p) {
    PStmt **stmtList = NULL;
    while (!check(p, T_END) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    eat(p, T_END, "Expected 'end' after block");
    PStmt *block = NewBlockStmt(p->gc, previous(p), stmtList);
    return block;
}

static PStmt *rToEndOrElseBlockStmt(Parser *p, bool *hasElse) {
    PStmt **stmtList = NULL;

    while ((!check(p, T_END) && !check(p, T_ELSE)) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    if (check(p, T_ELSE)) {
        eat(p, T_ELSE, "Expected 'else' ");
        *hasElse = true;
    } else if (check(p, T_END)) {
        eat(p, T_END, "Expected 'end'");
        *hasElse = false;
    }

    Token *op = previous(p);

    PStmt *block = NewBlockStmt(p->gc, op, stmtList);
    if (block == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create block statement"
        );
        return NULL;
    }
    return block;
}
static PStmt *rIfStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_THEN, "Expected then after if expression");
    bool hasElse = false;
    PStmt *thenBranch = rToEndOrElseBlockStmt(p, &hasElse);
    PStmt *elseBranch = NULL;
    if (hasElse) {
        elseBranch = rToEndBlockStmt(p);
    }

    PStmt *ifStmt = NewIfStmt(p->gc, op, cond, thenBranch, elseBranch);
    if (ifStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create if statement"
        );
        return NULL;
    }

    return ifStmt;
}

static PStmt *rWhileStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_DO, "Expected do after while expression");
    PStmt *body = rToEndBlockStmt(p);
    PStmt *whileStmt = NewWhileStmt(p->gc, op, cond, body);

    if (whileStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create while statement"
        );
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
        eat(p, T_SEMICOLON, "Expected ';' after empty return");
    }

    PStmt *retStmt = NewReturnStmt(p->gc, op, value);
    if (retStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create return statement"
        );
        return NULL;
    }

    return retStmt;
}

static PStmt *rBreakStmt(Parser *p) {
    Token *op = previous(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, "Expected ';'");
    }

    PStmt *breakStmt = NewBreakStmt(p->gc, op);

    if (breakStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create break statement"
        );
        return NULL;
    }

    return breakStmt;
}

static PStmt *rFuncStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, "Expected function name");
    eat(p, T_LEFT_PAREN, "Expected '(' after function name");
    Token **params = NULL;
    u64 paramCount = 0;
    if (!check(p, T_RIGHT_PAREN)) {
        do {
            Token *parm = eat(p, T_IDENT, "Expected param");
            arrput(params, parm);
            paramCount++;
        } while (matchOne(p, T_COMMA));
    }
    eat(p, T_RIGHT_PAREN, "Expected ')' after expression list");
    PStmt *body = rToEndBlockStmt(p);

    PStmt *funcStmt = NewFuncStmt(p->gc, name, params, body, paramCount);
    if (funcStmt == NULL) {
        fatalError(
            p, name,
            "Internal Memory Error : Failed to create function statement"
        );
        return NULL;
    }

    return funcStmt;
}

static PStmt *rImportStmt(Parser *p) {
    Token *op = previous(p);
    Token *name = eat(p, T_IDENT, "Expected Import Name");
    PExpr *pathExpr = rExpression(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, "Semicolon"); // Error should never occur
    }

    PStmt *importStmt = NewImportStmt(p->gc, op, name, pathExpr);
    if (importStmt == NULL) {
        fatalError(
            p, op, "Internal Memory Error : Failed to create import statement"
        );
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
