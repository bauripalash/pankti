#include "include/ast.h"
#include "include/alloc.h"
#include "include/token.h"
#include <stdio.h>

PExpr * NewBinaryExpr(PExpr * left, Token * op, PExpr * right){
	PExpr * e = PCreate(PExpr);
	e->type = EXPR_BINARY;
	e->exp.EBinary.left = left;
	e->exp.EBinary.opr = op;
	e->exp.EBinary.right = right;

	return e;
}

PExpr * NewUnary(Token * op, PExpr * right){
	PExpr * e = PCreate(PExpr);
	e->type = EXPR_UNARY;
	e->exp.EUnary.opr = op;
	e->exp.EUnary.right = right;
	return e;
}


PExpr * NewLiteral(Token * op, ExpLitType type){
	PExpr * e = PCreate(PExpr);
	e->type = EXPR_LITERAL;
	e->exp.ELiteral.op = op;
	e->exp.ELiteral.type = type;
	return e;
}

PExpr * NewGrouping(PExpr * expr){
	PExpr * e = PCreate(PExpr);
	e->type = EXPR_GROUPING;
	e->exp.EGrouping.expr = expr;
	return e;
}

void AstPrint(PExpr * expr){
	if (expr == NULL) {
		printf("Null Invalid Expression\n");
		return;
	}


	switch (expr->type) {
		case EXPR_BINARY:{
			printf("Binary: {\n L: {");
			AstPrint(expr->exp.EBinary.left);
			printf("} Op: {");
			PrintOpToken(expr->exp.EBinary.opr);
			printf("} R: {");
			AstPrint(expr->exp.EBinary.right);
			printf("}\n}");
			break;
		}
		case EXPR_UNARY:{
			printf("Unary : {\n Op: {");
			PrintOpToken(expr->exp.EUnary.opr);
			printf("} R: {");
			AstPrint(expr->exp.EUnary.right);
			printf("}");
			break;
		}
		case EXPR_LITERAL: {
			printf("%s", expr->exp.ELiteral.op->lexeme);
			break;
		} 
		default:printf("Unknown Expression to print\n");break;
	}
}
