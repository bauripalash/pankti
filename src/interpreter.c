#include "include/interpreter.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/object.h"
#include "include/token.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdlib.h>

static PObj * evaluate(PInterpreter * it, PExpr * expr);
PInterpreter * NewInterpreter(PExpr * prog){
	PInterpreter * it = PCreate(PInterpreter);
	it->program = prog;
	return it;
}
void FreeInterpreter(PInterpreter * it){
	if (it == NULL) {
		return;
	}

	free(it);
}
void Interpret(PInterpreter * it){
	PObj * obj = evaluate(it, it->program);
	PrintObject(obj);

}

static PObj * vLiteral(PInterpreter * it, PExpr * expr){
	PObj * litObj;
	switch (expr->exp.ELiteral.type) {
		case EXP_LIT_NUM: {
			litObj = NewObject(OT_NUM);
			litObj->v.num = atof(expr->exp.ELiteral.op->lexeme);
			break;
		}
		case EXP_LIT_STR:{
			litObj = NewObject(OT_STR);
			litObj->v.str = expr->exp.ELiteral.op->lexeme;
			break;
		}
		case EXP_LIT_BOOL:{
			bool bvalue = false;
			if (StrEqual(expr->exp.ELiteral.op->lexeme, "true")) {
				bvalue = true;
			} else {
				bvalue = false; //error check;
			}

			litObj = NewObject(OT_BOOL);
			litObj->v.bl = bvalue;
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
	PObj * result;
	switch (expr->exp.EBinary.opr->type) {
		case T_PLUS:{
			double value = l->v.num + r->v.num;
			result = NewNumberObj(value);
			break;
		}
		case T_ASTR:{
			double value = l->v.num * r->v.num;
			result = NewNumberObj(value);
			break;
		}
		default: result = NewNumberObj(-1);break;
	}

	return result;
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
		default:break;
	}

	return result;
}

static PObj * evaluate(PInterpreter * it, PExpr * expr){
	switch (expr->type) {
		case EXPR_LITERAL: return vLiteral(it, expr);
		case EXPR_BINARY: return vBinary(it, expr);
		case EXPR_UNARY: return vUnary(it, expr);
		default:break;
	}

	return NULL;
}
