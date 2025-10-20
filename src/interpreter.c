#include "include/interpreter.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/token.h"
#include "include/gc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static PObj *execute(PInterpreter *it, PStmt *stmt, PEnv *env);
static PObj *evaluate(PInterpreter *it, PExpr *expr, PEnv *env);
static PObj *execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret);

PInterpreter *NewInterpreter(Pgc * gc, PStmt **prog) {
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

    if (it->env != NULL) {
        FreeEnv(it->env);
    }

    free(it);
}
void Interpret(PInterpreter *it) {
    for (int i = 0; i < arrlen(it->program); i++) {
    	execute(it, it->program[i], it->env);
    }

    //FreeObject(it->gc, obj);
}
static void error(PInterpreter *it, void *tok, char *msg) {
    CoreError(it->core, -1, msg);
}
static PObj *vLiteral(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *litObj;
    switch (expr->exp.ELiteral.type) {
    case EXP_LIT_NUM: {
        double value = atof(expr->exp.ELiteral.op->lexeme);
        litObj = NewNumberObj(it->gc, value);
        break;
    }
    case EXP_LIT_STR: {
        litObj = NewStrObject(it->gc, expr->exp.ELiteral.op->lexeme);
        break;
    }
    case EXP_LIT_BOOL: {
        bool bvalue = false;
        if (expr->exp.ELiteral.op->type == T_TRUE) {
            bvalue = true;
        }
        litObj = NewBoolObj(it->gc, bvalue);
        break;
    }
    case EXP_LIT_NIL: {
        litObj = NewObject(it->gc, OT_NIL);
        break;
    }
    }

    return litObj;
}

static PObj *vBinary(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *l = evaluate(it, expr->exp.EBinary.left, env);
    PObj *r = evaluate(it, expr->exp.EBinary.right, env);
    PObj *result = NULL;
    switch (expr->exp.EBinary.opr->type) {
    case T_PLUS: {
        if (l->type != OT_NUM || r->type != OT_NUM) {
            error(it, NULL, "Plus operation can only be done with numbers;");
            break;
        }
        double value = l->v.num + r->v.num;
        result = NewNumberObj(it->gc, value);
        break;
    }
    case T_ASTR: {
        double value = l->v.num * r->v.num;
        result = NewNumberObj(it->gc, value);
        break;
    }
    case T_MINUS: {
        double value = l->v.num - r->v.num;
        result = NewNumberObj(it->gc, value);
        break;
    }
    case T_SLASH: {
        double value = l->v.num / r->v.num;
        result = NewNumberObj(it->gc, value);
        break;
    }
    case T_EQEQ: {
        result = NewBoolObj(it->gc, isObjEqual(l, r));
        break;
    }
    case T_BANG_EQ: {
        result = NewBoolObj(it->gc, !isObjEqual(l, r));
        break;
    }
    case T_GT: {
        result = NewBoolObj(it->gc, l->v.num > r->v.num);
        break;
    }
    case T_GTE:
        result = NewBoolObj(it->gc, l->v.num >= r->v.num);
        break;
    case T_LT:
        result = NewBoolObj(it->gc, l->v.num < r->v.num);
        break;
    case T_LTE:
        result = NewBoolObj(it->gc, l->v.num <= r->v.num);
        break;
    default:
        result = NewNumberObj(it->gc, -1);
        break;
    }

    return result;
}

static PObj *vVariable(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *v = EnvGetValue(env, expr->exp.EVariable.name->hash);
    if (v == NULL) {
        error(it, NULL, "Undefined variable");
        printf("at line %ld\n", expr->exp.EVariable.name->line);
        return NewNilObject(it->gc);
    }
    return v;
}

static PObj *vAssignment(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *value = evaluate(it, expr->exp.EAssign.value, env);
    if (EnvSetValue(env, expr->exp.EAssign.name->hash, value)) {
        return value;
    } else {
        error(it, NULL, "Undefined variable assignment");
        printf("at line %ld\n", expr->exp.EAssign.name->line);
        return NULL;
    }
}

static PObj *vUnary(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *r = evaluate(it, expr->exp.EUnary.right, env);
    PObj *result;

    switch (expr->exp.EUnary.opr->type) {
    case T_MINUS: {
        double value = -r->v.num;
        result = NewNumberObj(it->gc, value);
        break;
    }
    case T_BANG: {
        result = NewBoolObj(it->gc, !IsObjTruthy(r));
        break;
    }
    default: {
        result = NewNilObject(it->gc);
        break;
    }
    }

    return result;
}

static PObj *vLogical(PInterpreter *it, PExpr *expr, PEnv *env) {
    PObj *left = evaluate(it, expr->exp.ELogical.left, env);
    Token *op = expr->exp.ELogical.op;

    if (op->type == T_OR) {
        if (IsObjTruthy(left)) {
            return NewBoolObj(it->gc, true);
        } else {
            PObj *right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsObjTruthy(right)) {
                return NewBoolObj(it->gc, true);
            } else {
                return NewBoolObj(it->gc, false);
            }
        }
    } else if (op->type == T_AND) {
        if (!IsObjTruthy(left)) {
            return NewBoolObj(it->gc, false);
        } else {
            PObj *right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsObjTruthy(right)) {
                return NewBoolObj(it->gc, true);
            } else {
                return NewBoolObj(it->gc, false);
            }
        }
    }

    PrintToken(expr->exp.ELogical.op);
    error(it, NULL, "Invalid logical operation");
    return NULL;
}

static PObj *handleCall(PInterpreter *it, PObj *func, PObj **args, int count) {
    struct OFunction *f = &func->v.OFunction;
    PEnv *fnEnv = NewEnv((PEnv *)f->env);
    for (int i = 0; i < count; i++) {
        EnvPutValue(fnEnv, f->params[i]->hash, args[i]);
    }

    PObj *evalOut = execBlock(it, f->body, fnEnv, true);

    if (evalOut != NULL) {
        if (evalOut->type == OT_RET) {
            return evalOut->v.OReturn.rvalue;
        }
    } else {
        evalOut = NewNilObject(it->gc);
    }
    // printf("return->\n");

    // FreeEnv(fnEnv);

    return evalOut;
}

static PObj *vCall(PInterpreter *it, PExpr *expr, PEnv *env) {
    struct ECall *ec = &expr->exp.ECall;
    PObj *co = evaluate(it, ec->callee, env);
    if (co->type != OT_FNC) {
        error(it, NULL, "Can only call functions");
        return NULL;
    }

    if (co->v.OFunction.paramCount != ec->argCount) {
        error(it, NULL, "Func param count != call arg count");
        return NULL;
    }

    PObj **args = NULL;

    for (int i = 0; i < ec->argCount; i++) {
        arrput(args, evaluate(it, ec->args[i], env));
    }

    return handleCall(it, co, args, ec->argCount);
}

static PObj *evaluate(PInterpreter *it, PExpr *expr, PEnv *env) {
    if (expr == NULL) {
        return NULL;
    }
    switch (expr->type) {
    case EXPR_LITERAL:
        return vLiteral(it, expr, env);
    case EXPR_BINARY:
        return vBinary(it, expr, env);
    case EXPR_UNARY:
        return vUnary(it, expr, env);
    case EXPR_VARIABLE:
        return vVariable(it, expr, env);
    case EXPR_ASSIGN:
        return vAssignment(it, expr, env);
    case EXPR_LOGICAL:
        return vLogical(it, expr, env);
    case EXPR_CALL:
        return vCall(it, expr, env);
    case EXPR_GROUPING:
        return evaluate(it, expr->exp.EGrouping.expr, env);
    }

    return NULL;
}

static PObj *execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret) {
    PStmt **stmtList = stmt->stmt.SBlock.stmts;
    if (stmtList == NULL) {
        return NewNilObject(it->gc);
    }
    PObj *obj = NULL;
    // PEnv * ogEnv = it->env;
    // it->env = env;

    PEnv *blockEnv = NewEnv(env);
    for (int i = 0; i < arrlen(stmtList); i++) {
        obj = execute(it, stmtList[i], blockEnv);
        if (ret && obj->type == OT_RET) {
            return obj;
        }

        if (obj->type == OT_BRK) {
            return obj;
        }
    }

    // it->env = ogEnv;
    return obj;
}

static PObj *vsBlock(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PObj *obj = execBlock(it, stmt, env, true);
    return obj;
}

static PObj *vsLet(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PObj *value = evaluate(it, stmt->stmt.SLet.expr, env);
    EnvPutValue(env, stmt->stmt.SLet.name->hash, value);
    return value;
}

static PObj *vsPrint(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PObj *obj = evaluate(it, stmt->stmt.SPrint.value, env);
    PrintObject(obj);
    return obj;
}

static PObj *vsExprStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return evaluate(it, stmt->stmt.SExpr.expr, env);
}

static PObj *vsIfStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PObj *cond = evaluate(it, stmt->stmt.SIf.cond, env);
    if (IsObjTruthy(cond)) {
        return execute(it, stmt->stmt.SIf.thenBranch, env);
    } else {
        if (stmt->stmt.SIf.elseBranch != NULL) {
            return execute(it, stmt->stmt.SIf.elseBranch, env);
        }
    }

    return NewNilObject(it->gc);
}

static PObj *vsWhileStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PObj *obj;
    while (IsObjTruthy(evaluate(it, stmt->stmt.SWhile.cond, env))) {
        obj = execute(it, stmt->stmt.SWhile.body, env);
        if (obj->type == OT_BRK) {
            break;
        }
    }

    return obj;
}

static PObj *vsReturnStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    struct SReturn *ret = &stmt->stmt.SReturn;
    PObj *value = evaluate(it, ret->value, env);

    PObj *retValue = NewReturnObject(it->gc, value);
    return retValue;
}

static PObj *vsBreakStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return NewBreakObject(it->gc);
}

static PObj *vsFuncStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    struct SFunc *fs = &stmt->stmt.SFunc;
    PObj *f = NewFuncObject(
        it->gc, fs->name, fs->params, fs->body, NewEnv(env), fs->paramCount
    );
    EnvPutValue(env, fs->name->hash, f);
    return NewNilObject(it->gc);

}

static PObj *execute(PInterpreter *it, PStmt *stmt, PEnv *env) {
    switch (stmt->type) {
    case STMT_PRINT:
        return vsPrint(it, stmt, env);
        break;
    case STMT_EXPR:
        return vsExprStmt(it, stmt, env);
        break;
    case STMT_LET:
        return vsLet(it, stmt, env);
        break;
    case STMT_BLOCK:
        return vsBlock(it, stmt, env);
        break;
    case STMT_IF:
        return vsIfStmt(it, stmt, env);
        break;
    case STMT_WHILE:
        return vsWhileStmt(it, stmt, env);
        break;
    case STMT_RETURN:
        return vsReturnStmt(it, stmt, env);
        break;
    case STMT_BREAK:
        return vsBreakStmt(it, stmt, env);
    case STMT_FUNC:
        return vsFuncStmt(it, stmt, env);
    default:
        error(it, NULL, "Unknown statement found!");
    }

    return NewNilObject(it->gc);
}
