#include "include/interpreter.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/token.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static PValue execute(PInterpreter *it, PStmt *stmt, PEnv *env);
static PValue evaluate(PInterpreter *it, PExpr *expr, PEnv *env);
static PValue execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret);

PInterpreter *NewInterpreter(Pgc *gc, PStmt **prog) {
    PInterpreter *it = PCreate(PInterpreter);
    it->program = prog;
    it->core = NULL;
    it->env = NewEnv(NULL);
    it->gc = gc;
    return it;
}
void FreeInterpreter(PInterpreter *it) {
    if (it == NULL) {
        return;
    }

    // if (it->env != NULL) {
    FreeEnv(it->env);
    //}

    PFree(it);
}
void Interpret(PInterpreter *it) {
    for (int i = 0; i < arrlen(it->program); i++) {
        execute(it, it->program[i], it->env);
    }

    // FreeObject(it->gc, obj);
}
static void error(PInterpreter *it, void *tok, char *msg) {
    CoreError(it->core, -1, msg);
}
static PValue vLiteral(PInterpreter *it, PExpr *expr, PEnv *env) {
    switch (expr->exp.ELiteral.type) {
        case EXP_LIT_NUM: {
            double value = atof(expr->exp.ELiteral.op->lexeme);
            return MakeNumber(value);
        }
        case EXP_LIT_STR: {
            PObj * litObj = NewStrObject(it->gc, expr->exp.ELiteral.op->lexeme);
			return MakeObject(litObj);
        }
        case EXP_LIT_BOOL: {
            bool bvalue = false;
            if (expr->exp.ELiteral.op->type == T_TRUE) {
                bvalue = true;
            }
            //litObj = NewBoolObj(it->gc, bvalue);
			return MakeBool(bvalue);
        }
        case EXP_LIT_NIL: {
            //litObj = NewObject(it->gc, OT_NIL);
			return MakeNil();
        }
		default: return MakeNil();
    }

    //return litObj;
}

static PValue vBinary(PInterpreter *it, PExpr *expr, PEnv *env) {
    PValue l = evaluate(it, expr->exp.EBinary.left, env);
    PValue r = evaluate(it, expr->exp.EBinary.right, env);
    //if (l == NULL) {
    //    error(it, NULL, "Left hand side expression is invalid");
    //    return NULL;
    //}

    //if (r == NULL) {
    //    error(it, NULL, "Right hand side expression is invalid");
    //    return NULL;
    //}
    switch (expr->exp.EBinary.opr->type) {
        case T_PLUS: {
            if (l.type != VT_NUM || r.type != VT_NUM) {
                error(
                    it, NULL, "Plus operation can only be done with numbers;"
                );
                break;
            }
            double value = l.v.num + r.v.num;
            //result = NewNumberObj(it->gc, value);
			return MakeNumber(value);
            break;
        }
        case T_ASTR: {
            double value = l.v.num * r.v.num;
            //result = NewNumberObj(it->gc, value);
			return MakeNumber(value);
            break;
        }
        case T_MINUS: {
            double value = l.v.num - r.v.num;
            //result = NewNumberObj(it->gc, value);
			return MakeNumber(value);
            break;
        }
        case T_SLASH: {
            double value = l.v.num / r.v.num;
            //result = NewNumberObj(it->gc, value);
			return MakeNumber(value);
            break;
        }
        case T_EQEQ: {
            //result = NewBoolObj(it->gc, IsValueEqual(&l, &r));
			return MakeBool(IsValueEqual(&l, &r));
            break;
        }
        case T_BANG_EQ: {
			//result = NewBoolObj(it->gc, !IsValueEqual(&l, &r));
			return MakeBool(!IsValueEqual(&l, &r));

            break;
        }
        case T_GT: {
            //result = NewBoolObj(it->gc, l.v.num > r.v.num);
			return MakeBool(l.v.num > r.v.num);
            break;
        }
        case T_GTE: return MakeBool(l.v.num >= r.v.num);
        case T_LT: return MakeBool(l.v.num < r.v.num);
        case T_LTE: return MakeBool(l.v.num <= r.v.num);
        default: return MakeBool(-1);
    }

	return MakeNil();

}

static PValue vVariable(PInterpreter *it, PExpr *expr, PEnv *env) {
	bool found = false;
    PValue v = EnvGetValue(env, expr->exp.EVariable.name->hash, &found);
    if (!found) {
        char errmsg[512];
        snprintf(
            errmsg, 512, "Found Undefined variable '%s' at line %ld",
            expr->exp.EVariable.name->lexeme, expr->exp.EVariable.name->line
        );
        error(it, NULL, errmsg);
        return MakeNil();
    }
    return v;
}

static PValue vAssignment(PInterpreter *it, PExpr *expr, PEnv *env) {
    PValue value = evaluate(it, expr->exp.EAssign.value, env);
    if (EnvSetValue(
            env, expr->exp.EAssign.name->exp.EVariable.name->hash, value
        )) {
        return value;
    } else {
        error(it, NULL, "Undefined variable assignment");
        printf(
            "at line %ld\n", expr->exp.EAssign.name->exp.EVariable.name->line
        );
        return MakeNil();
    }
}

static PValue vUnary(PInterpreter *it, PExpr *expr, PEnv *env) {
	PValue r = evaluate(it, expr->exp.EUnary.right, env);

    switch (expr->exp.EUnary.opr->type) {
        case T_MINUS: {
            double value = -r.v.num;
            //result = NewNumberObj(it->gc, value);
			return MakeNumber(value);
            break;
        }
        case T_BANG: {
            //result = NewBoolObj(it->gc, !IsObjTruthy(r));
			return MakeBool(!IsValueTruthy(&r));
            break;
        }
        default: {
            //result = NewNilObject(it->gc);
			return MakeNil();
            break;
        }
    }

}

static PValue vLogical(PInterpreter *it, PExpr *expr, PEnv *env) {
    PValue left = evaluate(it, expr->exp.ELogical.left, env);
    Token *op = expr->exp.ELogical.op;

    if (op->type == T_OR) {
        if (IsValueTruthy(&left)) {
            //return NewBoolObj(it->gc, true);
			return MakeBool(true);
        } else {
            PValue right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsValueTruthy(&right)) {
                return MakeBool(true);
            } else {
                return MakeBool(false);

            }
        }
    } else if (op->type == T_AND) {
        if (!IsValueTruthy(&left)) {
            return MakeBool(false);
        } else {
        	PValue right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsValueTruthy(&right)) {
                return MakeBool(true);
            } else {
                return MakeBool(false);
            }
        }
    }

    PrintToken(expr->exp.ELogical.op);
    error(it, NULL, "Invalid logical operation");
    return MakeNil();
}

static PValue handleCall(PInterpreter *it, PObj *func, PValue *args, int count) {
    struct OFunction *f = &func->v.OFunction;
    PEnv *fnEnv = NewEnv((PEnv *)f->env);
    for (int i = 0; i < count; i++) {
        EnvPutValue(fnEnv, f->params[i]->hash, args[i]);
    }

    PValue evalOut = execBlock(it, f->body, fnEnv, true);

	if (evalOut.type == VT_OBJ) {
		PObj * objValue = evalOut.v.obj;
		if (objValue->type == OT_RET) {
			FreeEnv(fnEnv);
			return objValue->v.OReturn.rvalue;
			//MakeObject(objValue->v.OReturn.rvalue);
		}
	}

    FreeEnv(fnEnv);

    return evalOut;
}

static PValue vCall(PInterpreter *it, PExpr *expr, PEnv *env) {
    struct ECall *ec = &expr->exp.ECall;
    PValue co = evaluate(it, ec->callee, env);
    if (co.type != VT_OBJ) {
        error(it, NULL, "Can only call functions");
        return MakeNil();
    }

	if (co.type == VT_OBJ) {
		if (co.v.obj->type != OT_FNC) {
			error(it, NULL, "Can only call functions");
	        return MakeNil();
		}
	}

	PObj * callObj = ValueAsObj(co);
    if (callObj->v.OFunction.paramCount != ec->argCount) {
        error(it, NULL, "Func param count != call arg count");
        return MakeNil();
    }

	PValue argStack[16];
    PValue *args = NULL;

	if (ec->argCount > 16) {
		//printf("Using Heap for Args\n");
		args = PCalloc(16, sizeof(*args));
	}else{
		//printf("Using Stack for Args\n");
		args = argStack;
	}

    for (int i = 0; i < ec->argCount; i++) {
        //arrput(args, evaluate(it, ec->args[i], env));
		args[i] = evaluate(it, ec->args[i], env);
    }

    PValue value = handleCall(it, callObj, args, ec->argCount);
	if (ec->argCount > 16) {
		PFree(args);
	}
    return value;
}

static PValue evaluate(PInterpreter *it, PExpr *expr, PEnv *env) {
    if (expr == NULL) {
        return MakeNil();
    }
    switch (expr->type) {
        case EXPR_LITERAL: return vLiteral(it, expr, env);
        case EXPR_BINARY: return vBinary(it, expr, env);
        case EXPR_UNARY: return vUnary(it, expr, env);
        case EXPR_VARIABLE: return vVariable(it, expr, env);
        case EXPR_ASSIGN: return vAssignment(it, expr, env);
        case EXPR_LOGICAL: return vLogical(it, expr, env);
        case EXPR_CALL: return vCall(it, expr, env);
        case EXPR_GROUPING: return evaluate(it, expr->exp.EGrouping.expr, env);
    }

    return MakeNil();
}

static PValue execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret) {
    PStmt **stmtList = stmt->stmt.SBlock.stmts;
    if (stmtList == NULL) {
        return MakeNil();
    }
    PValue temp;
    // PEnv * ogEnv = it->env;
    // it->env = env;

    PEnv *blockEnv = NewEnv(env);
    for (int i = 0; i < arrlen(stmtList); i++) {
        temp = execute(it, stmtList[i], blockEnv);
        if (ret && temp.type == VT_OBJ && temp.v.obj->type == OT_RET) {

            FreeEnv(blockEnv);
            return temp;
        }

        if (temp.type == VT_OBJ && temp.v.obj->type == OT_RET) {

            FreeEnv(blockEnv);
            return temp;
        }
    }

    // it->env = ogEnv;
    FreeEnv(blockEnv);
    return temp;
}

static PValue vsBlock(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue value = execBlock(it, stmt, env, true);
    return value;
}

static PValue vsLet(PInterpreter *it, PStmt *stmt, PEnv *env) {
	PValue value = evaluate(it, stmt->stmt.SLet.expr, env);
    EnvPutValue(env, stmt->stmt.SLet.name->hash, value);
    return value;
}

static PValue vsPrint(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue value = evaluate(it, stmt->stmt.SPrint.value, env);
    PrintValue(&value);
    printf("\n");
    return MakeNil();
}

static PValue vsExprStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return evaluate(it, stmt->stmt.SExpr.expr, env);
}

static PValue vsIfStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue cond = evaluate(it, stmt->stmt.SIf.cond, env);
    if (IsValueTruthy(&cond)) {
        return execute(it, stmt->stmt.SIf.thenBranch, env);
    } else {
        if (stmt->stmt.SIf.elseBranch != NULL) {
            return execute(it, stmt->stmt.SIf.elseBranch, env);
        }
    }

    return MakeNil();
}

static PValue vsWhileStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
	PValue cond = evaluate(it,stmt->stmt.SWhile.cond, env);
    while (IsValueTruthy(&cond)) {
        PValue temp = execute(it, stmt->stmt.SWhile.body, env);
        if (temp.type == VT_OBJ && temp.v.obj->type == OT_BRK) {
            break;
        }

		cond = evaluate(it,stmt->stmt.SWhile.cond, env);
    }

    return MakeNil();
}

static PValue vsReturnStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    struct SReturn *ret = &stmt->stmt.SReturn;
    PValue value = evaluate(it, ret->value, env);

	PObj * retValue = NewReturnObject(it->gc, value);
    return MakeObject(retValue);
}

static PValue vsBreakStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return MakeObject(NewBreakObject(it->gc));
}

static PValue vsFuncStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    struct SFunc *fs = &stmt->stmt.SFunc;
    PObj *f = NewFuncObject(
        it->gc, fs->name, fs->params, fs->body, NewEnv(env), fs->paramCount
    );

    EnvPutValue(env, fs->name->hash, MakeObject(f));
    return MakeNil();
}

static PValue execute(PInterpreter *it, PStmt *stmt, PEnv *env) {
    switch (stmt->type) {
        case STMT_PRINT: return vsPrint(it, stmt, env); break;
        case STMT_EXPR: return vsExprStmt(it, stmt, env); break;
        case STMT_LET: return vsLet(it, stmt, env); break;
        case STMT_BLOCK: return vsBlock(it, stmt, env); break;
        case STMT_IF: return vsIfStmt(it, stmt, env); break;
        case STMT_WHILE: return vsWhileStmt(it, stmt, env); break;
        case STMT_RETURN: return vsReturnStmt(it, stmt, env); break;
        case STMT_BREAK: return vsBreakStmt(it, stmt, env);
        case STMT_FUNC: return vsFuncStmt(it, stmt, env);
        default: error(it, NULL, "Unknown statement found!");
    }

    return MakeNil();
}
