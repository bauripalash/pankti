#include "include/interpreter.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/env.h"
#include "include/object.h"
#include "include/token.h"
#include "include/core.h"
#include "external/stb/stb_ds.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static PObj * execute(PInterpreter * it, PStmt * stmt);
static PObj * evaluate(PInterpreter * it, PExpr * expr);
static PObj * execBlock(PInterpreter * it, PStmt * stmt, PEnv * env, bool ret);

PInterpreter * NewInterpreter(PStmt ** prog){
	PInterpreter * it = PCreate(PInterpreter);
	it->program = prog;
	it->core = NULL;
	it->env = NewEnv(NULL);
	return it;
}
void FreeInterpreter(PInterpreter * it){
	if (it == NULL) {
		return;
	}
	FreeEnv(it->env);

	free(it);
}
void Interpret(PInterpreter * it){
	for (int i = 0; i < arrlen(it->program); i++) {
		execute(it, it->program[i]);
	}

}
static void error(PInterpreter * it, void * tok, char * msg){
	CoreError(it->core, -1, msg);
}
static PObj * vLiteral(PInterpreter * it, PExpr * expr){
	PObj * litObj;
	switch (expr->exp.ELiteral.type) {
		case EXP_LIT_NUM: {
			double value = atof(expr->exp.ELiteral.op->lexeme);
			litObj = NewNumberObj(value);
			break;
		}
		case EXP_LIT_STR:{
			litObj = NewStrObject(expr->exp.ELiteral.op->lexeme);
			break;
		}
		case EXP_LIT_BOOL:{
			bool bvalue = false;
			if (expr->exp.ELiteral.op->type == T_TRUE) {
				bvalue = true;
			}
			litObj = NewBoolObj(bvalue);
			break;

		}
		case EXP_LIT_NIL:{
			litObj = NewObject(OT_NIL);
			break;
		}
	}

	return litObj;
}

static PObj * vBinary(PInterpreter * it, PExpr * expr){
	PObj * l = 	evaluate(it, expr->exp.EBinary.left);
	PObj * r = 	evaluate(it, expr->exp.EBinary.right);
	PObj * result = NULL;
	switch (expr->exp.EBinary.opr->type) {
		case T_PLUS:{
			if (l->type != OT_NUM || r->type != OT_NUM) {
				error(it, NULL, "Plus operation can only be done with numbers;");
				break;
			}
			double value = l->v.num + r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_ASTR:{
			double value = l->v.num * r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_MINUS:{
			double value = l->v.num - r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_SLASH:{
			double value = l->v.num / r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_EQEQ:{
			result = NewBoolObj(isObjEqual(l, r));
			break;
		}
		case T_BANG_EQ:{
			result = NewBoolObj(!isObjEqual(l, r));
			break;
		}
		case T_GT:{
			result = NewBoolObj(l->v.num > r->v.num);
			break;
		}
		default: result = NewNumberObj(-1);break;
	}

	return result;
}

static PObj * vVariable(PInterpreter * it, PExpr * expr){
	PObj * v = EnvGetValue(it->env, expr->exp.EVariable.name->hash);
	if (v == NULL) {
		error(it, NULL, "Undefined variable");
		printf("at line %ld\n", expr->exp.EVariable.name->line);
		return NewNilObject();
	}
	return v;
}

static PObj * vAssignment(PInterpreter *it, PExpr * expr){
	PObj * value = evaluate(it, expr->exp.EAssign.value);
	if (EnvSetValue(it->env, expr->exp.EAssign.name->hash, value)) {
		return value;
	}else{
		error(it, NULL, "Undefined variable assignment");
		printf("at line %ld\n", expr->exp.EAssign.name->line);
		return NULL;
	}

}

static PObj * vUnary(PInterpreter * it, PExpr * expr){
	PObj * r = evaluate(it, expr->exp.EUnary.right);
	PObj * result;

	switch (expr->exp.EUnary.opr->type) {
		case T_MINUS:{
			double value = -r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_BANG:{
			result = NewBoolObj(!IsObjTruthy(r));
			break;
		}
		default:{
			result = NewNilObject();
			break;
		}
	}

	return result;
}

static PObj * vLogical(PInterpreter * it, PExpr * expr){
	PObj * left = evaluate(it, expr->exp.ELogical.left);
	Token * op = expr->exp.ELogical.op;

	if (op->type == T_OR) {
		if (IsObjTruthy(left)) {
			return NewBoolObj(true);
		} else {
			PObj * right = evaluate(it, expr->exp.ELogical.right);
			if (IsObjTruthy(right)) {
				return NewBoolObj(true);
			}else{
				return NewBoolObj(false);
			}
		}
	} else if (op->type == T_AND){
		if (!IsObjTruthy(left)) {
			return NewBoolObj(false);
		} else {
			PObj * right = evaluate(it, expr->exp.ELogical.right);
			if (IsObjTruthy(right)) {
				return NewBoolObj(true);
			}else{
				return NewBoolObj(false);
			}
		}
	}

	PrintToken(expr->exp.ELogical.op);
	error(it, NULL, "Invalid logical operation");
	return NULL;

}

static PObj * evaluate(PInterpreter * it, PExpr * expr){
	switch (expr->type) {
		case EXPR_LITERAL: return vLiteral(it, expr);
		case EXPR_BINARY: return vBinary(it, expr);
		case EXPR_UNARY: return vUnary(it, expr);
		case EXPR_VARIABLE:return vVariable(it, expr);
		case EXPR_ASSIGN: return vAssignment(it, expr);
		case EXPR_LOGICAL:return vLogical(it, expr);
		default:break;
	}

	return NULL;
}

static PObj * execBlock(PInterpreter * it, PStmt * stmt, PEnv * env, bool ret){
	PObj * obj;
	PEnv * ogEnv = it->env;
	it->env = env;
	PStmt ** stmtList = stmt->stmt.SBlock.stmts;
	for (int i = 0; i < arrlen(stmtList); i++) {
		obj = execute(it, stmtList[i]);
	}
	it->env = ogEnv;
	return obj;
}

static PObj * vsBlock(PInterpreter * it, PStmt * stmt){
	PEnv * blockEnv = NewEnv(it->env);
	PObj * obj = execBlock(it, stmt, blockEnv, true);
	FreeEnv(blockEnv);
	return obj;

}

static PObj * vsLet(PInterpreter *it, PStmt * stmt){
	PObj * value = evaluate(it, stmt->stmt.SLet.expr);
	EnvPutValue(it->env, stmt->stmt.SLet.name->hash, value);
	return value;
}

static PObj* vsPrint(PInterpreter * it, PStmt * stmt){
	PObj * obj = evaluate(it, stmt->stmt.SPrint.value);
	PrintObject(obj);
	return obj;
}

static PObj * vsExprStmt(PInterpreter * it, PStmt * stmt){
	return evaluate(it, stmt->stmt.SExpr.expr);
}

static PObj * vsIfStmt(PInterpreter* it, PStmt * stmt){
	PObj * cond = evaluate(it, stmt->stmt.SIf.cond);
	if (IsObjTruthy(cond)) {
		return execute(it, stmt->stmt.SIf.thenBranch);
	} else{
		if (stmt->stmt.SIf.elseBranch != NULL) {
			return execute(it, stmt->stmt.SIf.elseBranch);
		}
	}

	return NewNilObject();

}

static PObj * vsWhileStmt(PInterpreter * it, PStmt * stmt){
	PObj * obj;
	while (IsObjTruthy(evaluate(it, stmt->stmt.SWhile.cond))) {
		obj = execute(it, stmt->stmt.SWhile.body);
	}

	return obj;
}



static PObj * vsReturnStmt(PInterpreter * it, PStmt * stmt){
	struct SReturn * ret = &stmt->stmt.SReturn;
	PObj * value = evaluate(it, ret->value);

	PObj * retValue = NewReturnObject(value);
	return retValue;
		
}

static PObj * execute(PInterpreter * it, PStmt * stmt){
	switch (stmt->type) {
		case STMT_PRINT: return vsPrint(it, stmt);break;
		case STMT_EXPR: return vsExprStmt(it, stmt);break;
		case STMT_LET: return vsLet(it, stmt);break;
		case STMT_BLOCK: return vsBlock(it, stmt);break;
		case STMT_IF: return vsIfStmt(it, stmt);break;
		case STMT_WHILE: return vsWhileStmt(it, stmt);break;
		case STMT_RETURN: return vsReturnStmt(it, stmt);break;
		default:error(it, NULL, "Unknown statement found!");
	}

	return NewNilObject();

}
