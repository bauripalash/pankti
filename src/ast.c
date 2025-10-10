#include "include/ast.h"
#include "include/alloc.h"
#include "include/token.h"
#include "external/stb/stb_ds.h"
#include <stdio.h>

PExpr * NewExpr(PExprType type){
	PExpr * e = PCreate(PExpr);
	//error handle
	
	e->type = type;
	return e;
}

PExpr * NewBinaryExpr(PExpr * left, Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_BINARY);
	e->exp.EBinary.left = left;
	e->exp.EBinary.opr = op;
	e->exp.EBinary.right = right;

	return e;
}

PExpr * NewUnary(Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_UNARY);
	e->exp.EUnary.opr = op;
	e->exp.EUnary.right = right;
	return e;
}


PExpr * NewLiteral(Token * op, ExpLitType type){
	PExpr * e = NewExpr(EXPR_LITERAL);
	e->exp.ELiteral.op = op;
	e->exp.ELiteral.type = type;
	return e;
}

PExpr * NewGrouping(PExpr * expr){
	PExpr * e = NewExpr(EXPR_GROUPING);
	e->exp.EGrouping.expr = expr;
	return e;
}


PExpr * NewVarExpr(Token * name){
	PExpr * e = NewExpr(EXPR_VARIABLE);
	e->exp.EVariable.name = name;
	return e;
}


PExpr * NewAssignment(Token * name, PExpr * value){
	PExpr * e = NewExpr(EXPR_ASSIGN);
	e->exp.EAssign.name = name;
	e->exp.EAssign.value = value;
	return e;
}

PStmt * NewStmt(PStmtType type){
	PStmt * p = PCreate(PStmt);
	//error handle
	p->type = type;
	return p;
}

PStmt * NewPrintStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_PRINT);
	s->stmt.SPrint.op = op;
	s->stmt.SPrint.value = value;
	return s;
}
PStmt * NewExprStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_EXPR);
	s->stmt.SExpr.op = op;
	s->stmt.SExpr.expr = value;
	return s;;
}


PStmt * NewLetStmt(Token * name, PExpr * value){
	PStmt * s = NewStmt(STMT_LET);
	s->stmt.SLet.name = name;
	s->stmt.SLet.expr = value;

	return s;
}


PStmt * NewBlockStmt(Token * op, PStmt ** stmts){
	PStmt * s = NewStmt(STMT_BLOCK);
	s->stmt.SBlock.op = op;
	s->stmt.SBlock.stmts = stmts;
	return s;
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
		case EXPR_GROUPING:{
			printf("Group : { E: \n");
			AstPrint(expr->exp.EGrouping.expr);
			printf("}");
			break;
		}
		case EXPR_VARIABLE:{
			printf("Var : { N: \n");
			PrintToken(expr->exp.EVariable.name);
			printf("}");
			break;
		}
		case EXPR_ASSIGN:{
			printf("Assign : { N: \n");
			PrintToken(expr->exp.EAssign.name);
			printf("} V: {\n");
			AstPrint(expr->exp.EAssign.value);
			printf("}");
			break;
		}
		default:printf("Unknown Expression to print\n");break;
	}
}

void AstStmtPrint(PStmt * stmt){
	switch (stmt->type) {
		case STMT_PRINT:{
			printf("Print : { E: \n");
			AstPrint(stmt->stmt.SPrint.value);
			printf("}");
			break;
		}

		case STMT_EXPR:{
			printf("Expr : { E: \n");
			AstPrint(stmt->stmt.SExpr.expr);
			printf("}");
			break;
		}
		case STMT_LET:{
			printf("Let : { N: \n");
			PrintToken(stmt->stmt.SLet.name);
			printf("} V: {\n");
			AstPrint(stmt->stmt.SLet.expr);
			printf("}\n}");
			break;
		}
		case STMT_BLOCK:{
			printf("Block : { S: { \n");
			for (int i = 0; i < arrlen(stmt->stmt.SBlock.stmts); i++) {
				printf("{");
				AstStmtPrint(stmt->stmt.SBlock.stmts[i]);
				printf("}");
			}
			printf("}}\n");

			break;
		}
		default:printf("Unknown stmt to print : %d\n" ,stmt->type );break;
	}
}
