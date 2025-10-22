#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ansicolors.h"
#include "../include/ast.h"
#include "../include/gc.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>

// ===================
// Creation Functions
// ===================

PStmt *NewStmt(Pgc *gc, PStmtType type) {
    PStmt *s = PCreate(PStmt);
    if (s == NULL) {
        return NULL;
    }
    s->type = type;
    s->next = gc->stmts;
    gc->stmts = s;
#if defined DEBUG_GC
    printf(
        TERMC_BLUE "[DEBUG] [GC] %p New Statement : %s\n" TERMC_RESET, s,
        StmtTypeToStr(type)
    );
#endif
    return s;
}

PStmt *NewPrintStmt(Pgc *gc, Token *op, PExpr *value) {
    PStmt *s = NewStmt(gc, STMT_PRINT);

    if (s == NULL) {
        return NULL;
    }
    s->stmt.SPrint.op = op;
    s->stmt.SPrint.value = value;
    return s;
}
PStmt *NewExprStmt(Pgc *gc, Token *op, PExpr *value) {
    PStmt *s = NewStmt(gc, STMT_EXPR);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SExpr.op = op;
    s->stmt.SExpr.expr = value;
    return s;
    ;
}

PStmt *NewLetStmt(Pgc *gc, Token *name, PExpr *value) {
    PStmt *s = NewStmt(gc, STMT_LET);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SLet.name = name;
    s->stmt.SLet.expr = value;

    return s;
}

PStmt *NewBlockStmt(Pgc *gc, Token *op, PStmt **stmts) {
    PStmt *s = NewStmt(gc, STMT_BLOCK);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SBlock.op = op;
    s->stmt.SBlock.stmts = stmts;
    return s;
}

PStmt *NewIfStmt(Pgc *gc, Token *op, PExpr *cond, PStmt *then, PStmt *elseB) {
    PStmt *s = NewStmt(gc, STMT_IF);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SIf.op = op;
    s->stmt.SIf.cond = cond;
    s->stmt.SIf.thenBranch = then;
    s->stmt.SIf.elseBranch = elseB;
    return s;
}

PStmt *NewWhileStmt(Pgc *gc, Token *op, PExpr *cond, PStmt *body) {
    PStmt *s = NewStmt(gc, STMT_WHILE);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SWhile.op = op;
    s->stmt.SWhile.cond = cond;
    s->stmt.SWhile.body = body;
    return s;
}

PStmt *NewReturnStmt(Pgc *gc, Token *op, PExpr *value) {
    PStmt *s = NewStmt(gc, STMT_RETURN);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SReturn.op = op;
    s->stmt.SReturn.value = value;
    return s;
}

PStmt *NewBreakStmt(Pgc *gc, Token *op) {
    PStmt *s = NewStmt(gc, STMT_BREAK);
    if (s == NULL) {
        return NULL;
    }
    s->stmt.SBreak.op = op;
    return s;
}

PStmt *
NewFuncStmt(Pgc *gc, Token *name, Token **params, PStmt *body, int count) {
    PStmt *s = NewStmt(gc, STMT_FUNC);
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

static inline void freeBaseStmt(Pgc *gc, PStmt *stmt) {
    if (stmt != NULL) {
        free(stmt);
    }
}

void FreeStmt(Pgc *gc, PStmt *s) {
    if (s == NULL) {
        return;
    }
#if defined DEBUG_GC
    printf(
        TERMC_GREEN "[DEBUG] [GC] Freeing Statement : %s\n" TERMC_RESET,
        StmtTypeToStr(s->type)
    );
#endif
    switch (s->type) {
    case STMT_EXPR: {
        FreeExpr(gc, s->stmt.SExpr.expr);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_LET: {
        // should we do this?
        FreeExpr(gc, s->stmt.SLet.expr);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_PRINT: {
        // what if the variable is still in env and used later?
        FreeExpr(gc, s->stmt.SPrint.value);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_RETURN: {
        FreeExpr(gc, s->stmt.SReturn.value);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_FUNC: {
        struct SFunc *fs = &s->stmt.SFunc;
        arrfree(fs->params);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_BLOCK: {
        struct SBlock *blk = &s->stmt.SBlock;
        arrfree(blk->stmts);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_IF: {
        FreeExpr(gc, s->stmt.SIf.cond);
        freeBaseStmt(gc, s);
        break;
    }
    case STMT_WHILE: {
        FreeExpr(gc, s->stmt.SWhile.cond);
        freeBaseStmt(gc, s);
        break;
    }

    case STMT_BREAK: {
        freeBaseStmt(gc, s);
        break;
    }
    }
}
