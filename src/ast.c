#include "include/ast.h"
#include "include/alloc.h"
#include "include/token.h"
#include "include/ansicolors.h"
#include "external/stb/stb_ds.h"
#include <stdio.h>

PExpr * NewExpr(PExprType type){
	PExpr * e = PCreate(PExpr);
	if (e == NULL) {
		return NULL;
	}
	e->type = type;
	return e;
}


PExpr * NewBinaryExpr(PExpr * left, Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_BINARY);
	if (e == NULL) {
		return NULL;
	}
	e->exp.EBinary.left = left;
	e->exp.EBinary.opr = op;
	e->exp.EBinary.right = right;

	return e;
}

PExpr * NewUnary(Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_UNARY);
	if (e == NULL) {
		return NULL;
	}
	e->exp.EUnary.opr = op;
	e->exp.EUnary.right = right;
	return e;
}


PExpr * NewLiteral(Token * op, ExpLitType type){
	PExpr * e = NewExpr(EXPR_LITERAL);
	if (e == NULL) {
		return NULL;
	}
	e->exp.ELiteral.op = op;
	e->exp.ELiteral.type = type;
	return e;
}

PExpr * NewGrouping(PExpr * expr){
	PExpr * e = NewExpr(EXPR_GROUPING);
	if (e == NULL) {
		return NULL;
	}
	e->exp.EGrouping.expr = expr;
	return e;
}


PExpr * NewVarExpr(Token * name){
	PExpr * e = NewExpr(EXPR_VARIABLE);
	if (e == NULL) {
		return NULL;
	}
	e->exp.EVariable.name = name;
	return e;
}


PExpr * NewAssignment(Token * name, PExpr * value){
	PExpr * e = NewExpr(EXPR_ASSIGN);
	if (e == NULL) {
		return NULL;
	}
	e->exp.EAssign.name = name;
	e->exp.EAssign.value = value;
	return e;
}


PExpr * NewLogical(PExpr * left, Token * op, PExpr * right){
	PExpr * e = NewExpr(EXPR_LOGICAL);
	if (e == NULL) {
		return NULL;
	}
	e->exp.ELogical.op = op;
	e->exp.ELogical.left = left;
	e->exp.ELogical.right = right;
	return e;
}


PExpr * NewCallExpr(Token * op, PExpr * callee, PExpr ** args, int count){
	PExpr * e = NewExpr(EXPR_CALL);
	if (e == NULL) {
		return NULL;
	}
	e->exp.ECall.op = op;
	e->exp.ECall.callee = callee;
	e->exp.ECall.args = args;
	e->exp.ECall.argCount = count;
	return e;
}


PStmt * NewStmt(PStmtType type){
	PStmt * s = PCreate(PStmt);
	if (s == NULL) {
		return NULL;
	}
	s->type = type;
	return s;
}

PStmt * NewPrintStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_PRINT);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SPrint.op = op;
	s->stmt.SPrint.value = value;
	return s;
}
PStmt * NewExprStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_EXPR);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SExpr.op = op;
	s->stmt.SExpr.expr = value;
	return s;;
}


PStmt * NewLetStmt(Token * name, PExpr * value){
	PStmt * s = NewStmt(STMT_LET);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SLet.name = name;
	s->stmt.SLet.expr = value;

	return s;
}


PStmt * NewBlockStmt(Token * op, PStmt ** stmts){
	PStmt * s = NewStmt(STMT_BLOCK);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SBlock.op = op;
	s->stmt.SBlock.stmts = stmts;
	return s;
}


PStmt * NewIfStmt(Token * op, PExpr * cond, PStmt * then, PStmt * elseB){
	PStmt * s = NewStmt(STMT_IF);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SIf.op = op;
	s->stmt.SIf.cond = cond;
	s->stmt.SIf.thenBranch = then;
	s->stmt.SIf.elseBranch = elseB;
	return s;
}

PStmt * NewWhileStmt(Token * op, PExpr * cond, PStmt * body){
	PStmt * s = NewStmt(STMT_WHILE);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SWhile.op = op;
	s->stmt.SWhile.cond = cond;
	s->stmt.SWhile.body = body;
	return s;
}

PStmt * NewReturnStmt(Token * op, PExpr * value){
	PStmt * s = NewStmt(STMT_RETURN);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SReturn.op = op;
	s->stmt.SReturn.value = value;
	return s;
}


PStmt * NewBreakStmt(Token * op){
	PStmt * s = NewStmt(STMT_BREAK);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SBreak.op = op;
	return s;
}


PStmt * NewFuncStmt(Token * name, Token ** params, PStmt * body, int count){
	PStmt * s = NewStmt(STMT_FUNC);
	if (s == NULL) {
		return NULL;
	}
	s->stmt.SFunc.name = name;
	s->stmt.SFunc.params = params;
	s->stmt.SFunc.body = body;
	s->stmt.SFunc.paramCount = count;
	return s;
}

static void printIndent(int indent){
	for (int i = 0; i < indent; i++) {
		printf("  ");
	}
}

void AstPrintLiteral(PExpr * expr){
	if (expr == NULL) {
		return;
	}

	if (expr->type != EXPR_LITERAL) {
		return;
	}

	struct ELiteral * lit = &expr->exp.ELiteral;
	switch (lit->type) {
		case EXP_LIT_NUM:{ 
			printf(
				TERMC_YELLOW
				"Number(" 
				TERMC_RESET 
				"%s" 
				TERMC_YELLOW 
				")" 
				TERMC_RESET, lit->op->lexeme);
			printf("\n");
			break;
		}
		case EXP_LIT_STR:{
			printf(
				TERMC_YELLOW
				"String("
				TERMC_RESET
				"%s"
				TERMC_YELLOW
				")"
				TERMC_RESET, lit->op->lexeme
			);
			printf("\n");
			break;

		}
		default:return;
	}
}

void AstPrint(PExpr * expr, int indent){
	if (expr == NULL) {
		printf("Invalid Expression!\n");
		return;
	}

	printIndent(indent);

	switch (expr->type) {
		case EXPR_BINARY:{
			printf(
				TERMC_PURPLE
				"BinaryExpr("
				TERMC_RESET
				"%s"
				TERMC_PURPLE
				")\n" 
				TERMC_RESET, TokTypeToStr(expr->exp.EBinary.opr->type));

			AstPrint(expr->exp.EBinary.left, indent + 1);
			AstPrint(expr->exp.EBinary.right, indent + 1);
			break;
		}
		case EXPR_UNARY:{
			printf(
				TERMC_GREEN
				"Unary("
				TERMC_RESET
				"%s"
				TERMC_GREEN
				")\n"
				TERMC_RESET, TokTypeToStr(expr->exp.EBinary.opr->type));

			AstPrint(expr->exp.EUnary.right, indent + 1);
			break;
		}
		case EXPR_LITERAL: {
			AstPrintLiteral(expr);
			break;
		} 
		case EXPR_GROUPING:{
			printf(
				TERMC_GREEN 
				"Group\n" 
				TERMC_RESET);

			AstPrint(expr->exp.EGrouping.expr, indent + 1);
			break;
		}
		case EXPR_VARIABLE:{
			printf(
				TERMC_RED 
				"Var(" 
				TERMC_GREEN 
				"%s" 
				TERMC_RED 
				")\n" 
				TERMC_RESET, expr->exp.EVariable.name->lexeme);

			break;
		}
		case EXPR_ASSIGN:{
			printf(
				TERMC_YELLOW 
				"Assign(" 
				TERMC_GREEN 
				"%s" 
				TERMC_YELLOW 
				")\n" 
				TERMC_RESET, expr->exp.EAssign.name->lexeme);

			AstPrint(expr->exp.EAssign.value, indent + 1);
			break;
		}
		case EXPR_LOGICAL:{
			printf("Logical : {}\n");
			break;
		}
		case EXPR_CALL:{
			printf("Call : <%d> {}\n", expr->exp.ECall.argCount);
			break;
		}

	}
}

void AstStmtPrint(PStmt * stmt, int indent){
	if (stmt == NULL) {
		printf("Invalid Statement\n");
		return;
	}
	printIndent(indent);
	switch (stmt->type) {
		case STMT_PRINT:{
			printf("Print [\n");
			AstPrint(stmt->stmt.SPrint.value,indent + 1);
			printIndent(indent);
			printf("]\n");
			break;
		}

		case STMT_EXPR:{
			printf("Expr [\n");
			AstPrint(stmt->stmt.SExpr.expr,indent + 1);
			printf("\n]\n");
			break;
		}
		case STMT_LET:{
			printf(
				"Let (" 
				TERMC_GREEN 
				"%s" 
				TERMC_RESET 
				") [\n", stmt->stmt.SLet.name->lexeme);

			AstPrint(stmt->stmt.SLet.expr, indent + 1);
			printf("\n]\n");
			break;
		}
		case STMT_BLOCK:{
			printf("Block [\n");
			for (int i = 0; i < arrlen(stmt->stmt.SBlock.stmts); i++) {
				AstStmtPrint(stmt->stmt.SBlock.stmts[i],indent + 1);
			}
			printIndent(indent);
			printf("]\n");

			break;
		}
		case STMT_IF:{
			printf("If [\n");
			printIndent(indent + 1);
			printf("Cond {\n");
			AstPrint(stmt->stmt.SIf.cond,indent + 2);
			printIndent(indent + 1);
			printf("}\n");
			printIndent(indent + 1);
			printf("Then {\n");
			AstStmtPrint(stmt->stmt.SIf.thenBranch,indent+2);
			printIndent(indent + 1);
			printf("}\n");
			printIndent(indent + 1);
			printf("Else {\n");
			if (stmt->stmt.SIf.elseBranch != NULL) {
				AstStmtPrint(stmt->stmt.SIf.elseBranch,indent + 2);
			}
			printIndent(indent+1);
			printf("}\n");
			printIndent(indent);
			printf("]\n");

			break;
		}
		case STMT_WHILE:{
			printf("While : { C: {");
			AstPrint(stmt->stmt.SWhile.cond,indent);
			printf("} {...} }");
			break;
		}

		case STMT_RETURN:{
			printf("Return : { V: {");
			AstPrint(stmt->stmt.SReturn.value,indent);
			printf("}}\n");
			break;
		}
		case STMT_BREAK:{
			printf("Break : {}\n");
			break;
		}
		case STMT_FUNC:{
			printf("Func : <%s> {}\n",stmt->stmt.SFunc.name->lexeme);
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
		case STMT_FUNC: return "Func Stmt";break;
	
	}

	return "Unknown Statement";
}
