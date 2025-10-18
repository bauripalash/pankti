#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ast.h"
#include "../include/token.h"
#include <stdio.h>

PExpr *NewExpr(PExprType type) {
    PExpr *e = PCreate(PExpr);
    if (e == NULL) {
        return NULL;
    }
    e->type = type;
    return e;
}

PExpr *NewBinaryExpr(PExpr *left, Token *op, PExpr *right) {
    PExpr *e = NewExpr(EXPR_BINARY);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EBinary.left = left;
    e->exp.EBinary.opr = op;
    e->exp.EBinary.right = right;

    return e;
}

PExpr *NewUnary(Token *op, PExpr *right) {
    PExpr *e = NewExpr(EXPR_UNARY);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EUnary.opr = op;
    e->exp.EUnary.right = right;
    return e;
}

PExpr *NewLiteral(Token *op, ExpLitType type) {
    PExpr *e = NewExpr(EXPR_LITERAL);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ELiteral.op = op;
    e->exp.ELiteral.type = type;
    return e;
}

PExpr *NewGrouping(PExpr *expr) {
    PExpr *e = NewExpr(EXPR_GROUPING);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EGrouping.expr = expr;
    return e;
}

PExpr *NewVarExpr(Token *name) {
    PExpr *e = NewExpr(EXPR_VARIABLE);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EVariable.name = name;
    return e;
}

PExpr *NewAssignment(Token *name, PExpr *value) {
    PExpr *e = NewExpr(EXPR_ASSIGN);
    if (e == NULL) {
        return NULL;
    }
    e->exp.EAssign.name = name;
    e->exp.EAssign.value = value;
    return e;
}

PExpr *NewLogical(PExpr *left, Token *op, PExpr *right) {
    PExpr *e = NewExpr(EXPR_LOGICAL);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ELogical.op = op;
    e->exp.ELogical.left = left;
    e->exp.ELogical.right = right;
    return e;
}

PExpr *NewCallExpr(Token *op, PExpr *callee, PExpr **args, int count) {
    PExpr *e = NewExpr(EXPR_CALL);
    if (e == NULL) {
        return NULL;
    }
    e->exp.ECall.op = op;
    e->exp.ECall.callee = callee;
    e->exp.ECall.args = args;
    e->exp.ECall.argCount = count;
    return e;
}
