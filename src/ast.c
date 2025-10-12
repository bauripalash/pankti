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


PExpr * NewLogical(PExpr * left, Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_LOGICAL);
	e->exp.ELogical.op = op;
	e->exp.ELogical.left = left;
	e->exp.ELogical.right = right;
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


PStmt * NewIfStmt(Token * op, PExpr * cond, PStmt * then, PStmt * elseB){
	PStmt * s = NewStmt(STMT_IF);
	s->stmt.SIf.op = op;
	s->stmt.SIf.cond = cond;
	s->stmt.SIf.thenBranch = then;
	s->stmt.SIf.elseBranch = elseB;
	return s;
}

PStmt * NewWhileStmt(Token * op, PExpr * cond, PStmt * body){
	PStmt * s = NewStmt(STMT_WHILE);
	s->stmt.SWhile.op = op;
	s->stmt.SWhile.cond = cond;
	s->stmt.SWhile.body = body;
	return s;
}

PStmt * NewReturnStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_RETURN);
	s->stmt.SReturn.op = op;
	s->stmt.SReturn.value = value;
	return s;
}


PStmt * NewBreakStmt(Token * op){
	PStmt * s = NewStmt(STMT_BREAK);
	s->stmt.SBreak.op = op;
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
		case EXPR_LOGICAL:{
			printf("Logical : {}\n");
			break;
		}

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
			printf("SExpr : { E: \n");
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
		case STMT_IF:{
			printf("If : { C: {");
			AstPrint(stmt->stmt.SIf.cond);
			printf("} : { T: \n");
			AstStmtPrint(stmt->stmt.SIf.thenBranch);
			if (stmt->stmt.SIf.elseBranch != NULL) {

				printf("} : { E: \n");
				AstStmtPrint(stmt->stmt.SIf.elseBranch);

			}
			printf("}}");

			break;
		}
		case STMT_WHILE:{
			printf("While : { C: {");
			AstPrint(stmt->stmt.SWhile.cond);
			printf("} {...} }");
			break;
		}

		case STMT_RETURN:{
			printf("Return : { V: {");
			AstPrint(stmt->stmt.SReturn.value);
			printf("}}\n");
			break;
		}
		case STMT_BREAK:{
			printf("Break : {}\n");
			break;
		}
	}
}


char * StmtTypeToStr(PStmtType type){
	switch (type) {
		case STMT_RETURN: return "Return Stmt";break;
		case STMT_WHILE: return "While Stmt";break;
		case STMT_IF: return "If Stmt";break;
		case STMT_BLOCK: return "Block Stmt";break;
		case STMT_LET: return "Let Stmt";break;
		case STMT_EXPR: return "Expr Stmt";break;
		case STMT_PRINT: return "Print Stmt";break;
		case STMT_BREAK: return "Break Stmt";break;
	
	}

	return "";
}
