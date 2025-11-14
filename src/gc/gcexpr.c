#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ast.h"
#include "../include/gc.h"
#include "../include/token.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stdio.h>

// ===================
// Creation Functions
// ===================

PExpr *NewExpr(Pgc *gc, PExprType type, Token *op) {
    PExpr *e = PCreate(PExpr);
    if (e == NULL) {
        return NULL;
    }
    e->type = type;
    e->op = op;
    return e;
}

PExpr *NewBinaryExpr(Pgc *gc, PExpr *left, Token *op, PExpr *right) {
    PExpr *e = NewExpr(gc, EXPR_BINARY, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EBinary.left = left;
    e->exp.EBinary.op = op;
    e->exp.EBinary.right = right;

    return e;
}

PExpr *NewUnary(Pgc *gc, Token *op, PExpr *right) {
    PExpr *e = NewExpr(gc, EXPR_UNARY, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EUnary.op = op;
    e->exp.EUnary.right = right;
    return e;
}

PExpr *NewLiteral(Pgc *gc, Token *op, ExpLitType type) {
    PExpr *e = NewExpr(gc, EXPR_LITERAL, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ELiteral.op = op;
    e->exp.ELiteral.type = type;
    return e;
}

PExpr *NewGrouping(Pgc *gc, Token *op, PExpr *expr) {
    PExpr *e = NewExpr(gc, EXPR_GROUPING, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EGrouping.op = op;
    e->exp.EGrouping.expr = expr;
    return e;
}

PExpr *NewVarExpr(Pgc *gc, Token *name) {
    PExpr *e = NewExpr(gc, EXPR_VARIABLE, name);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EVariable.name = name;
    return e;
}

PExpr *NewAssignment(Pgc *gc, Token *op, PExpr *name, PExpr *value) {
    PExpr *e = NewExpr(gc, EXPR_ASSIGN, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EAssign.op = op;
    e->exp.EAssign.name = name;
    e->exp.EAssign.value = value;
    return e;
}

PExpr *NewLogical(Pgc *gc, PExpr *left, Token *op, PExpr *right) {
    PExpr *e = NewExpr(gc, EXPR_LOGICAL, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ELogical.op = op;
    e->exp.ELogical.left = left;
    e->exp.ELogical.right = right;
    return e;
}

PExpr *NewCallExpr(Pgc *gc, Token *op, PExpr *callee, PExpr **args, int count) {
    PExpr *e = NewExpr(gc, EXPR_CALL, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ECall.op = op;
    e->exp.ECall.callee = callee;
    e->exp.ECall.args = args;
    e->exp.ECall.argCount = count;
    return e;
}

PExpr *NewArrayExpr(Pgc *gc, Token *op, PExpr **items, int count) {
    PExpr *e = NewExpr(gc, EXPR_ARRAY, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EArray.op = op;
    e->exp.EArray.items = items;
    e->exp.EArray.count = count;
    return e;
}

PExpr *NewMapExpr(Pgc *gc, Token *op, PExpr **items, int count) {
    PExpr *e = NewExpr(gc, EXPR_MAP, op);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EMap.op = op;
    e->exp.EMap.etable = items;
    e->exp.EMap.count = count;
    return e;
}

PExpr *NewSubscriptExpr(Pgc *gc, Token *op, PExpr *value, PExpr *index) {
    PExpr *e = NewExpr(gc, EXPR_SUBSCRIPT, op);
    if (e == NULL) {
        return NULL;
    }

    e->exp.ESubscript.op = op;
    e->exp.ESubscript.value = value;
    e->exp.ESubscript.index = index;
    return e;
}

PExpr *NewModgetExpr(Pgc *gc, Token *op, PExpr *module, Token *child) {
    PExpr *e = NewExpr(gc, EXPR_MODGET, op);
    if (e == NULL) {
        return NULL;
    }

    e->exp.EModget.op = op;
    e->exp.EModget.module = module;
    e->exp.EModget.child = child;
    return e;
}
// ===================
// Freeing Functions
// ===================

// Free the base Expr Struct with NULL check
static inline void freeBaseExpr(Pgc *gc, PExpr *expr) {
    if (expr != NULL) {
        PFree(expr);
        expr = NULL;
    }
}

void FreeExpr(Pgc *gc, PExpr *e) {
    // Freeing Expressions
    // Never Free tokens. Token are directly referenced from Lexer
    if (e == NULL) {
        return;
    }

    switch (e->type) {
        case EXPR_CALL: {
            FreeExpr(gc, e->exp.ECall.callee);
            int count = e->exp.ECall.argCount;
            for (int i = 0; i < count; i++) {
                PExpr *ex = arrpop(e->exp.ECall.args);
                FreeExpr(gc, ex);
            }
            arrfree(e->exp.ECall.args);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_LOGICAL: {
            FreeExpr(gc, e->exp.ELogical.left);
            FreeExpr(gc, e->exp.ELogical.right);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_MODGET: {
            FreeExpr(gc, e->exp.EModget.module);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_ASSIGN: {
            FreeExpr(gc, e->exp.EAssign.name);
            FreeExpr(gc, e->exp.EAssign.value);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_VARIABLE: {
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_GROUPING: {
            FreeExpr(gc, e->exp.EGrouping.expr);
            freeBaseExpr(gc, e);
            break;
        }

        case EXPR_LITERAL: {
            freeBaseExpr(gc, e);
            if (e->exp.ELiteral.type == EXP_LIT_STR) {
                if (e->exp.ELiteral.value.svalue != NULL) {
                    PFree(e->exp.ELiteral.value.svalue);
                }
            }
            break;
        }
        case EXPR_UNARY: {
            FreeExpr(gc, e->exp.EUnary.right);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_BINARY: {
            FreeExpr(gc, e->exp.EBinary.left);
            FreeExpr(gc, e->exp.EBinary.right);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_ARRAY: {
            int count = e->exp.EArray.count;
            for (int i = 0; i < count; i++) {
                FreeExpr(gc, arrpop(e->exp.EArray.items));
            }
            arrfree(e->exp.EArray.items);

            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_MAP: {
            struct EMap *map = &e->exp.EMap;
            for (int i = 0; i < map->count; i++) {
                FreeExpr(gc, arrpop(map->etable));
            }

            arrfree(map->etable);
            freeBaseExpr(gc, e);
            break;
        }
        case EXPR_SUBSCRIPT: {
            FreeExpr(gc, e->exp.ESubscript.value);
            FreeExpr(gc, e->exp.ESubscript.index);
            freeBaseExpr(gc, e);
            break;
        }
    }
}
