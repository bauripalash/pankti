#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ast.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>

// ===================
// Creation Functions
// ===================

PStmt *NewStmt(PStmtType type) {
    PStmt *s = PCreate(PStmt);
    if (s == NULL) {
        return NULL;
    }
    s->type = type;
    return s;
}

PStmt *NewPrintStmt(Token *op, PExpr *value) {
    PStmt *s = NewStmt(STMT_PRINT);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SPrint.op = op;
    s->stmt.SPrint.value = value;
    return s;
}
PStmt *NewExprStmt(Token *op, PExpr *value) {
    PStmt *s = NewStmt(STMT_EXPR);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SExpr.op = op;
    s->stmt.SExpr.expr = value;
    return s;
    ;
}

PStmt *NewLetStmt(Token *name, PExpr *value) {
    PStmt *s = NewStmt(STMT_LET);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SLet.name = name;
    s->stmt.SLet.expr = value;

    return s;
}

PStmt *NewBlockStmt(Token *op, PStmt **stmts) {
    PStmt *s = NewStmt(STMT_BLOCK);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SBlock.op = op;
    s->stmt.SBlock.stmts = stmts;
    return s;
}

PStmt *NewIfStmt(Token *op, PExpr *cond, PStmt *then, PStmt *elseB) {
    PStmt *s = NewStmt(STMT_IF);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SIf.op = op;
    s->stmt.SIf.cond = cond;
    s->stmt.SIf.thenBranch = then;
    s->stmt.SIf.elseBranch = elseB;
    return s;
}

PStmt *NewWhileStmt(Token *op, PExpr *cond, PStmt *body) {
    PStmt *s = NewStmt(STMT_WHILE);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SWhile.op = op;
    s->stmt.SWhile.cond = cond;
    s->stmt.SWhile.body = body;
    return s;
}

PStmt *NewReturnStmt(Token *op, PExpr *value) {
    PStmt *s = NewStmt(STMT_RETURN);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SReturn.op = op;
    s->stmt.SReturn.value = value;
    return s;
}

PStmt *NewBreakStmt(Token *op) {
    PStmt *s = NewStmt(STMT_BREAK);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SBreak.op = op;
    return s;
}

PStmt *NewFuncStmt(Token *name, Token **params, PStmt *body, int count) {
    PStmt *s = NewStmt(STMT_FUNC);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SFunc.name = name;
    s->stmt.SFunc.params = params;
    s->stmt.SFunc.body = body;
    s->stmt.SFunc.paramCount = count;
    return s;
}

// ===================
// Freeing Functions
// ===================

static inline void freeBaseStmt(PStmt *stmt) {
    if (stmt != NULL) {
        free(stmt);
    }
}

void FreeStmt(PStmt *s) {
    if (s == NULL) {
        return;
    }
    switch (s->type) {
    case STMT_EXPR: {
        FreeExpr(s->stmt.SExpr.expr);
        freeBaseStmt(s);
        break;
    }
    case STMT_LET: {
        // should we do this?
        FreeExpr(s->stmt.SLet.expr);
        freeBaseStmt(s);
        break;
    }
    case STMT_PRINT: {
        // what if the variable is still in env and used later?
        FreeExpr(s->stmt.SLet.expr);
        freeBaseStmt(s);
        break;
    }
    default:
        break;
    }
}
