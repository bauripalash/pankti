#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ast.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>


// ===================
// Creation Functions
// ===================


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

// ===================
// Freeing Functions
// ===================

// Free the base Expr Struct with NULL check
static inline void freeBaseExpr(PExpr * expr){
	if (expr != NULL) {
		free(expr);
	}
}



void FreeExpr(PExpr * e){
	// Freeing Expressions
	// Never Free tokens. Token are directly referenced from Lexer
	if (e == NULL) {
		return;
	}

	switch (e->type) {
		case EXPR_CALL:{
			FreeExpr(e->exp.ECall.callee);
			int count = e->exp.ECall.argCount;
			for (int i = 0; i < count; i++) {
				FreeExpr(e->exp.ECall.args[i]);
			}
			freeBaseExpr(e);
			break;
		}
		case EXPR_LOGICAL:{
			FreeExpr(e->exp.ELogical.left);
			FreeExpr(e->exp.ELogical.right);
			freeBaseExpr(e);
			break;
		}
		case EXPR_ASSIGN:{
			FreeExpr(e->exp.EAssign.value);
			freeBaseExpr(e);
			break;
		}
		case EXPR_VARIABLE:{
			freeBaseExpr(e);
			break;
		}
		case EXPR_GROUPING:{
			FreeExpr(e->exp.EGrouping.expr);
			freeBaseExpr(e);
			break;
		}

		case EXPR_LITERAL:{
			freeBaseExpr(e);
			break;
		}
		case EXPR_UNARY:{
			FreeExpr(e->exp.EUnary.right);
			freeBaseExpr(e);
			break;
		}
		case EXPR_BINARY:{
			FreeExpr(e->exp.EBinary.left);
			FreeExpr(e->exp.EBinary.right);
			freeBaseExpr(e);
			break;
		}
	}

}
