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

typedef enum ExType {
    ET_SIMPLE,
    ET_BREAK,
    ET_RETURN,
} ExType;

typedef struct ExResult {
    PValue value;
    ExType type;
} ExResult;

static char *ExResultToStr(ExType t) {
    switch (t) {
        case ET_SIMPLE: return "Simple";
        case ET_BREAK: return "Break";
        case ET_RETURN: return "return";
    }
}

static inline ExResult ExSimple(PValue v) {
    ExResult er;
    er.type = ET_SIMPLE;
    er.value = v;
    return er;
}

static inline ExResult ExBreak() {
    ExResult er;
    er.type = ET_BREAK;
    er.value = MakeNil();
    return er;
}

static inline ExResult ExReturn(PValue v) {
    ExResult er;
    er.type = ET_RETURN;
    er.value = v;
    return er;
}

static ExResult execute(PInterpreter *it, PStmt *stmt, PEnv *env);
static PValue evaluate(PInterpreter *it, PExpr *expr, PEnv *env);
static ExResult execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret);

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
static void error(PInterpreter *it, Token *tok, const char *msg) {
    CoreError(it->core, tok, msg);
}
static PValue vLiteral(PInterpreter *it, PExpr *expr, PEnv *env) {
    switch (expr->exp.ELiteral.type) {
        case EXP_LIT_NUM: {
            double value = atof(expr->exp.ELiteral.op->lexeme);
            return MakeNumber(value);
        }
        case EXP_LIT_STR: {
            PObj *litObj = NewStrObject(it->gc, expr->exp.ELiteral.op->lexeme);
            return MakeObject(litObj);
        }
        case EXP_LIT_BOOL: {
            bool bvalue = false;
            if (expr->exp.ELiteral.op->type == T_TRUE) {
                bvalue = true;
            }
            return MakeBool(bvalue);
        }
        case EXP_LIT_NIL: {
            return MakeNil();
        }
        default: return MakeNil();
    }

    // return litObj;
}

static PValue vBinary(PInterpreter *it, PExpr *expr, PEnv *env) {
    PValue l = evaluate(it, expr->exp.EBinary.left, env);
    PValue r = evaluate(it, expr->exp.EBinary.right, env);
    // if (l == NULL) {
    //     error(it, NULL, "Left hand side expression is invalid");
    //     return NULL;
    // }

    // if (r == NULL) {
    //     error(it, NULL, "Right hand side expression is invalid");
    //     return NULL;
    // }
    switch (expr->exp.EBinary.op->type) {
        case T_PLUS: {
            if (l.type != VT_NUM || r.type != VT_NUM) {
                error(
                    it, expr->exp.EBinary.op,
                    StrFormat(
                        "Plus operation can only be done with numbers but got "
                        "%s + %s",
                        ValueTypeToStr(&l), ValueTypeToStr(&r)
                    )
                );
                break;
            }
            double value = l.v.num + r.v.num;
            // result = NewNumberObj(it->gc, value);
            return MakeNumber(value);
            break;
        }
        case T_ASTR: {
            double value = l.v.num * r.v.num;
            // result = NewNumberObj(it->gc, value);
            return MakeNumber(value);
            break;
        }
        case T_MINUS: {
            double value = l.v.num - r.v.num;
            // result = NewNumberObj(it->gc, value);
            return MakeNumber(value);
            break;
        }
        case T_SLASH: {
            double value = l.v.num / r.v.num;
            // result = NewNumberObj(it->gc, value);
            return MakeNumber(value);
            break;
        }
        case T_EQEQ: {
            // result = NewBoolObj(it->gc, IsValueEqual(&l, &r));
            return MakeBool(IsValueEqual(&l, &r));
            break;
        }
        case T_BANG_EQ: {
            // result = NewBoolObj(it->gc, !IsValueEqual(&l, &r));
            return MakeBool(!IsValueEqual(&l, &r));

            break;
        }
        case T_GT: {
            // result = NewBoolObj(it->gc, l.v.num > r.v.num);
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
        error(
            it, expr->exp.EVariable.name,
            StrFormat(
                "Found Undefined variable '%s'",
                expr->exp.EVariable.name->lexeme
            )
        );
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
        error(it, expr->exp.EAssign.name->op, "Undefined variable assignment");
        return MakeNil();
    }
}

static PValue vUnary(PInterpreter *it, PExpr *expr, PEnv *env) {
    PValue r = evaluate(it, expr->exp.EUnary.right, env);

    switch (expr->exp.EUnary.op->type) {
        case T_MINUS: {
            double value = -r.v.num;
            // result = NewNumberObj(it->gc, value);
            return MakeNumber(value);
            break;
        }
        case T_BANG: {
            // result = NewBoolObj(it->gc, !IsObjTruthy(r));
            return MakeBool(!IsValueTruthy(&r));
            break;
        }
        default: {
            // result = NewNilObject(it->gc);
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
            // return NewBoolObj(it->gc, true);
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

static PValue
handleCall(PInterpreter *it, PObj *func, PValue *args, int count) {
    struct OFunction *f = &func->v.OFunction;
    PEnv *fnEnv = NewEnv((PEnv *)f->env);
    for (int i = 0; i < count; i++) {
        EnvPutValue(fnEnv, f->params[i]->hash, args[i]);
    }

    ExResult evalOut = execBlock(it, f->body, fnEnv, true);

    if (evalOut.type == ET_RETURN) {
        FreeEnv(fnEnv);
        return evalOut.value;
    }

    FreeEnv(fnEnv);

    return evalOut.value;
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

    PObj *callObj = ValueAsObj(co);
    if (callObj->v.OFunction.paramCount != ec->argCount) {
        error(it, NULL, "Func param count != call arg count");
        return MakeNil();
    }

    PValue argStack[16];
    PValue *args = NULL;

    if (ec->argCount > 16) {
        // printf("Using Heap for Args\n");
        args = PCalloc(16, sizeof(*args));
    } else {
        // printf("Using Stack for Args\n");
        args = argStack;
    }

    for (int i = 0; i < ec->argCount; i++) {
        // arrput(args, evaluate(it, ec->args[i], env));
        args[i] = evaluate(it, ec->args[i], env);
    }

    PValue value = handleCall(it, callObj, args, ec->argCount);
    if (ec->argCount > 16) {
        PFree(args);
    }
    return value;
}

static PValue vArray(PInterpreter *it, PExpr *expr, PEnv *env) {
    struct EArray *arr = &expr->exp.EArray;
    PValue *vitems = NULL;

    for (int i = 0; i < arr->count; i++) {
        arrput(vitems, evaluate(it, arr->items[i], env));
    }

    PObj *arrObj = NewArrayObject(it->gc, arr->op, vitems, arr->count);
    return MakeObject(arrObj);
}

static PValue vSubscript(PInterpreter *it, PExpr *expr, PEnv *env) {
    struct ESubscript *sub = &expr->exp.ESubscript;
    PValue subValue = evaluate(it, sub->value, env);

    if (subValue.type == VT_OBJ) {
        PObj *subObj = ValueAsObj(subValue);
        if (subObj->type == OT_ARR) {
            PValue indexValue = evaluate(it, sub->index, env);
            if (indexValue.type == VT_NUM) {
                double index = ValueAsNum(indexValue);
                struct OArray *arr = &subObj->v.OArray;
                if (index >= arr->count) {
                    error(it, NULL, "Index out of range");
                    return MakeNil();
                }

                return arr->items[(int)index];

            } else {
                error(it, NULL, "Array Subscript index must be a number");
                return MakeNil();
            }
        }
    }

    error(it, NULL, "Invalid Subscript. Subscript only works on array and ..");
    return MakeNil();
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
        case EXPR_ARRAY: return vArray(it, expr, env);
        case EXPR_SUBSCRIPT: return vSubscript(it, expr, env);
    }

    return MakeNil();
}

static ExResult execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret) {
    PStmt **stmtList = stmt->stmt.SBlock.stmts;
    if (stmtList == NULL) {
        return ExSimple(MakeNil());
    }
    ExResult temp;
    // PEnv * ogEnv = it->env;
    // it->env = env;

    PEnv *blockEnv = NewEnv(env);
    for (int i = 0; i < arrlen(stmtList); i++) {
        temp = execute(it, stmtList[i], blockEnv);
        // printf("Exec result -> %s\n", ExResultToStr(temp.type));
        if (ret && temp.type == ET_RETURN) {
            FreeEnv(blockEnv);
            return temp;
        }
        if (temp.type == ET_BREAK) {
            FreeEnv(blockEnv);
            return temp;
        }
    }

    // it->env = ogEnv;
    FreeEnv(blockEnv);
    return temp;
}

static ExResult vsBlock(PInterpreter *it, PStmt *stmt, PEnv *env) {
    // PValue value =
    return execBlock(it, stmt, env, true);
    // return value;
}

static ExResult vsLet(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue value = evaluate(it, stmt->stmt.SLet.expr, env);
    EnvPutValue(env, stmt->stmt.SLet.name->hash, value);
    return ExSimple(value);
}

static ExResult vsPrint(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue value = evaluate(it, stmt->stmt.SPrint.value, env);
    PrintValue(&value);
    printf("\n");
    return ExSimple(MakeNil());
}

static ExResult vsExprStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return ExSimple(evaluate(it, stmt->stmt.SExpr.expr, env));
}

static ExResult vsIfStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue cond = evaluate(it, stmt->stmt.SIf.cond, env);
    if (IsValueTruthy(&cond)) {
        return execute(it, stmt->stmt.SIf.thenBranch, env);
    } else {
        if (stmt->stmt.SIf.elseBranch != NULL) {
            return execute(it, stmt->stmt.SIf.elseBranch, env);
        }
    }

    return ExSimple(MakeNil());
}

static ExResult vsWhileStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    PValue cond = evaluate(it, stmt->stmt.SWhile.cond, env);
    while (IsValueTruthy(&cond)) {
        ExResult res = execute(it, stmt->stmt.SWhile.body, env);
        if (res.type == ET_BREAK) {
            break;
        }

        cond = evaluate(it, stmt->stmt.SWhile.cond, env);
    }

    return ExSimple(MakeNil());
}

static ExResult vsReturnStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    struct SReturn *ret = &stmt->stmt.SReturn;
    PValue value = evaluate(it, ret->value, env);
    return ExReturn(value);
    // PObj * retValue = NewReturnObject(it->gc, value);
    // return MakeObject(retValue);
}

static ExResult vsBreakStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    return ExBreak();
    // MakeObject(NewBreakObject(it->gc));
}

static ExResult vsFuncStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {

    struct SFunc *fs = &stmt->stmt.SFunc;
    PObj *f = NewFuncObject(
        it->gc, fs->name, fs->params, fs->body, NewEnv(env), fs->paramCount
    );

    EnvPutValue(env, fs->name->hash, MakeObject(f));
    return ExSimple(MakeNil());
}

static ExResult execute(PInterpreter *it, PStmt *stmt, PEnv *env) {
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

    return ExSimple(MakeNil());
}
