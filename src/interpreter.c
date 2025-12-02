#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/native.h"
#include "include/object.h"
#include "include/pstdlib.h"
#include "include/ptypes.h"
#include "include/token.h"
#include "include/utils.h"

#ifdef PANKTI_BUILD_DEBUG
#undef NDDEBUG
#endif

// Execution Result Type
typedef enum ExType {
    // Simple Result with value
    ET_SIMPLE,
    // Break statements
    ET_BREAK,
    // Return Statement with value
    ET_RETURN,
} ExType;

// Execution Result
typedef struct ExResult {
    // Execution Result Value (only for Simple and Return)
    PValue value;
    // Execution Result Type
    ExType type;
} ExResult;

// Create a Simple Execution Result
static inline ExResult ExSimple(PValue v) {
    ExResult er = {.type = ET_SIMPLE, .value = v};
    return er;
}

// Create a Break Execution Result
static inline ExResult ExBreak(void) {
    ExResult er = {.type = ET_BREAK, .value = MakeNil()};

    return er;
}

// Create a Return Execution Result
static inline ExResult ExReturn(PValue v) {
    ExResult er = {.type = ET_RETURN, .value = v};
    return er;
}

static PModule *NewModule(PInterpreter *it, char *name) {
    PModule *mod = PMalloc(sizeof(PModule));
    if (mod == NULL) {
        // throw error
        return NULL;
    }
    mod->pathname = StrDuplicate(name, (u64)strlen(name));
    // error check
    mod->env = NewEnv(it->gc, NULL);
    return mod;
}

static void PushModule(PInterpreter *it, PModule *mod) {
    arrput(it->mods, mod);
    it->modCount = (u64)arrlen(it->mods);
}

static void PushProxy(PInterpreter *it, u64 key, char *name, PModule *mod) {
    if (mod == NULL) {
        return; // error check
    }

    ModProxyEntry s = {.key = key, .name = name, .mod = mod};
    hmputs(it->proxyTable, s);
    it->proxyCount = (u64)hmlen(it->proxyTable);
}

// Execute Statement
static ExResult execute(PInterpreter *it, PStmt *stmt, PEnv *env);
// Evaluate expression
static PValue evaluate(PInterpreter *it, PExpr *expr, PEnv *env);
// Execute a Block
static ExResult execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret);

PInterpreter *NewInterpreter(Pgc *gc, PStmt **prog) {
    PInterpreter *it = PCreate(PInterpreter);
    it->program = prog;
    it->core = NULL;
    it->env = NewEnv(gc, NULL);
    it->gc = gc;
    it->proxyTable = NULL;
    it->proxyCount = 0;
    it->mods = NULL;
    it->modCount = 0;
    it->callDepth = 0;
    it->maxCallDepth = DEF_MAX_CALL_DEPTH;
    RegisterNatives(it, it->env);
    RegisterRootEnv(gc, it->env);
    return it;
}
void FreeInterpreter(PInterpreter *it) {
    if (it == NULL) {
        return;
    }

    if (it->env != NULL) {
        ReallyFreeEnv(it->env);
    }

    if (it->proxyCount > 0 && it->proxyTable != NULL) {
        hmfree(it->proxyTable);
    }

    if (it->modCount > 0 && it->mods != NULL) {
        for (int i = 0; i < it->modCount; i++) {
            PModule *mod = arrpop(it->mods);
            PFree(mod->pathname);
            ReallyFreeEnv(mod->env);
            PFree(mod);
        }

        arrfree(it->mods);
    }

    PFree(it);
}
void Interpret(PInterpreter *it) {
    u64 progCount = (u64)arrlen(it->program);
    for (u64 i = 0; i < progCount; i++) {
        execute(it, it->program[i], it->env);
        CollectGarbage(it->gc);
    }

    // FreeObject(it->gc, obj);
}

// Report a Error
static void error(PInterpreter *it, Token *tok, const char *msg) {
    CoreRuntimeError(it->core, tok, msg);
}

// Evaluate Literal Expression
// Numbers, Strings, Bools, Nil
static PValue vLiteral(PInterpreter *it, PExpr *expr, PEnv *env) {
    if (expr == NULL) {
        error(it, NULL, "Invalid literal found");
        return MakeNil();
    }
    assert(expr->type == EXPR_LITERAL);

    switch (expr->exp.ELiteral.type) {
        case EXP_LIT_NUM: {
            double value = expr->exp.ELiteral.value.nvalue;
            return MakeNumber(value);
        }
        case EXP_LIT_STR: {
            PObj *litObj = NewStrObject(
                it->gc, expr->op, expr->exp.ELiteral.value.svalue, false
            );
            return MakeObject(litObj);
        }
        case EXP_LIT_BOOL: {
            bool bvalue = expr->exp.ELiteral.value.bvalue;
            return MakeBool(bvalue);
        }
        case EXP_LIT_NIL: {
            return MakeNil();
        }
        default: return MakeNil();
    }
}

// Evaluate Binary Expression
// Basic Math: Addition, Substraction, Multiplication, Division
// Comparison: Equality Check, Not Equal,
// Greater Than, Greater than or Equal, Less Than, Less Than or Equal
static PValue vBinary(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_BINARY);

    PValue l = evaluate(it, expr->exp.EBinary.left, env);
    PValue r = evaluate(it, expr->exp.EBinary.right, env);

    switch (expr->exp.EBinary.op->type) {
        case T_PLUS: {
            if (IsValueNum(l) && IsValueNum(r)) {
                double value = ValueAsNum(l) + ValueAsNum(r);
                return MakeNumber(value);
            } else if ((IsValueObj(l) && ValueAsObj(l)->type == OT_STR) &&
                       (IsValueObj(r) && ValueAsObj(r)->type == OT_STR)) {
                bool ok = true;
                struct OString *ls = &ValueAsObj(l)->v.OString;
                u64 lsLen = (u64)strlen(ls->value);
                struct OString *rs = &ValueAsObj(r)->v.OString;
                u64 rsLen = (u64)strlen(rs->value);
                char *newStr = StrJoin(ls->value, lsLen, rs->value, rsLen, &ok);
                if (!ok) {
                    error(it, expr->op, "Fail to join string");
                    return MakeNil();
                }

                PObj *nsObj = NewStrObject(it->gc, expr->op, newStr, true);
                return MakeObject(nsObj);
            } else {
                error(
                    it, expr->op,
                    "Addition operation can only done when both values are "
                    "either numbers or strings"
                );
                return MakeNil();
            }
            break;
        }
            // Multiplication
        case T_ASTR: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op, "Multiplication can only be done with numbers"
                );
                return MakeNil();
            }
            double value = ValueAsNum(l) * ValueAsNum(r);
            return MakeNumber(value);
        }
        case T_EXPONENT: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op, "Multiplication can only be done with numbers"
                );
                return MakeNil();
            }

            double value = pow(ValueAsNum(l), ValueAsNum(r));
            return MakeNumber(value);
        }
        case T_MINUS: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op, "Substraction can only be done with numbers"
                );
                return MakeNil();
            }
            double value = ValueAsNum(l) - ValueAsNum(r);
            return MakeNumber(value);
            break;
        }
            // Division
        case T_SLASH: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(it, expr->op, "Division can only be done with numbers");
                return MakeNil();
            }

            if (ValueAsNum(r) == 0) {
                error(it, expr->op, "Division by zero");
                return MakeNil();
            }

            double value = ValueAsNum(l) / ValueAsNum(r);
            return MakeNumber(value);
            break;
        }
        case T_EQEQ: return MakeBool(IsValueEqual(l, r));
        case T_BANG_EQ: return MakeBool(!IsValueEqual(l, r));
        case T_GT: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op,
                    "Greater than operator can only be used with numbers"
                );
                return MakeNil();
            }
            return MakeBool(ValueAsNum(l) > ValueAsNum(r));
        }
        case T_GTE: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op,
                    "Greater than or equal to operator can only be used with "
                    "numbers"
                );
                return MakeNil();
            }
            return MakeBool(ValueAsNum(l) >= ValueAsNum(r));
        }
        case T_LT: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(it, expr->op, "Less than can only be used with numbers");
                return MakeNil();
            }
            return MakeBool(ValueAsNum(l) < ValueAsNum(r));
        }
        case T_LTE: {
            if (!IsValueNum(l) || !IsValueNum(r)) {
                error(
                    it, expr->op,
                    "Less than or equal to operator can only be used with "
                    "numbers"
                );
                return MakeNil();
            }
            return MakeBool(ValueAsNum(l) <= ValueAsNum(r));
        }
        default: return MakeNil();
    }

    return MakeNil();
}

// Evaluate a variable
// Returns the value of the variable from the `env` enviornment
static PValue vVariable(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_VARIABLE);
    bool found = false;
    struct EVariable *var = &expr->exp.EVariable;
    PValue v = EnvGetValue(env, var->name->hash, &found);

    if (!found) {
        error(
            it, var->name,
            StrFormat("Found Undefined variable '%s'", var->name->lexeme)
        );
        return MakeNil();
    }

    return v;
}

// Assignment Operation for Variables
// `name` = The actual target expression which must evaluate to Variable expr
// `val` = The new value to assign
static PValue varAssignment(
    PInterpreter *it, PExpr *name, PExpr *val, PEnv *env
) {
    assert(name->type == EXPR_VARIABLE);
    struct EVariable *target = &name->exp.EVariable;
    PValue vValue = evaluate(it, val, env);

    if (EnvSetValue(env, target->name->hash, vValue)) {
        return vValue;
    } else {
        error(it, target->name, "Undefined variable assignment");
        return MakeNil();
    }

    return MakeNil();
}

// Array Subscript Assignment
// myarr[0] = "NewValue"
static void arrAssignment(
    PInterpreter *it, PObj *arrObj, PExpr *index, PExpr *value, PEnv *env
) {
    assert(arrObj->type == OT_ARR);
    struct OArray *arr = &arrObj->v.OArray;
    PValue indexValue = evaluate(it, index, env);
    if (!IsValueNum(indexValue)) {
        error(it, index->op, "Array Objects can be indexed only with Integers");
        return;
    }

    double indexDouble = ValueAsNum(indexValue);

    if (!IsDoubleInt(indexDouble)) {
        error(it, index->op, "Array Objects can be indexed only with Integers");
        return;
    }

    int indexInt = (int)indexDouble;

    if (indexInt < 0 || indexInt >= arr->count) {
        error(it, index->op, "Index out of range");
        return;
    }

    PValue newValue = evaluate(it, value, env);
    if (!ArrayObjInsValue(arrObj, indexInt, newValue)) {
        error(it, index->op, "Internal Error : Failed to set value to array");
        return;
    }
}

static void mapAssignment(
    PInterpreter *it, PObj *mapObj, PExpr *keyExpr, PExpr *valueExpr, PEnv *env
) {
    assert(mapObj->type == OT_MAP);
    PValue keyValue = evaluate(it, keyExpr, env);

    if (!CanValueBeKey(keyValue)) {
        error(it, keyExpr->op, "Invalid key for map");
        return;
    }

    u64 keyHash = GetValueHash(keyValue, it->gc->timestamp);
    PValue value = evaluate(it, valueExpr, env);
    if (!MapObjSetValue(mapObj, keyValue, keyHash, value)) {
        error(it, keyExpr->op, "Internal Error : Failed to set value to map");
        return;
    }
}

static PValue subAssignment(
    PInterpreter *it, PExpr *targetExpr, PExpr *valueExpr, PEnv *env
) {
    assert(targetExpr->type == EXPR_SUBSCRIPT);
    struct ESubscript *sub = &targetExpr->exp.ESubscript;

    PValue coreValue = evaluate(it, sub->value, env);
    if (IsValueObj(coreValue)) {
        PObj *coreObj = ValueAsObj(coreValue);
        if (coreObj->type == OT_ARR) {
            arrAssignment(it, coreObj, sub->index, valueExpr, env);
            return MakeNil();
        } else if (coreObj->type == OT_MAP) {
            mapAssignment(it, coreObj, sub->index, valueExpr, env);
            return MakeNil();
        }
    }
    error(
        it, sub->value->op, "Subscript operation only valid for array and maps"
    );
    return MakeNil();
}

// Assignment Operation
// Setting new value to prexisting variable
// Replace items inside array
// Put New Key-Value pair to map or replace prexisting one, if key exists
static PValue vAssignment(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_ASSIGN);

    struct EAssign *assign = &expr->exp.EAssign;
    PExpr *target = assign->name;

    if (target->type == EXPR_VARIABLE) {
        return varAssignment(it, target, assign->value, env);
    } else if (target->type == EXPR_SUBSCRIPT) {
        return subAssignment(it, target, assign->value, env);
    }
    error(it, target->op, "Invalid assignment target");
    return MakeNil();
}

// Unary Operation
// -100 (Change sign of a number)
// !true (Logical NOT operation; Flip boolean)
static PValue vUnary(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_UNARY);

    PValue r = evaluate(it, expr->exp.EUnary.right, env);

    switch (expr->exp.EUnary.op->type) {
        case T_MINUS: {
            if (!IsValueNum(r)) {
                error(
                    it, expr->op,
                    "Unary negative operator can only be used with numbers"
                );
                return MakeNil();
            }
            double value = -ValueAsNum(r);
            return MakeNumber(value);
        }
        case T_BANG: {
            return MakeBool(!IsValueTruthy(r));
        }
        default: {
            error(it, expr->op, "Invalid Unary Operation");
            return MakeNil();
        }
    }
}

// Logical Operation : AND , OR
static PValue vLogical(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_LOGICAL);

    PValue left = evaluate(it, expr->exp.ELogical.left, env);
    Token *op = expr->exp.ELogical.op;

    if (op->type == T_OR) {
        if (IsValueTruthy(left)) {
            return MakeBool(true);
        } else {
            PValue right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsValueTruthy(right)) {
                return MakeBool(true);
            } else {
                return MakeBool(false);
            }
        }
    } else if (op->type == T_AND) {
        if (!IsValueTruthy(left)) {
            return MakeBool(false);
        } else {
            PValue right = evaluate(it, expr->exp.ELogical.right, env);
            if (IsValueTruthy(right)) {
                return MakeBool(true);
            } else {
                return MakeBool(false);
            }
        }
    }

    error(it, expr->op, "Invalid logical operation");
    return MakeNil();
}

// Evaluate a function call and return result (if any)
static PValue handleCall(
    PInterpreter *it, PObj *func, PValue *args, u64 count, PExpr *callExpr
) {
    assert(func->type == OT_FNC);
    struct OFunction *f = &func->v.OFunction;

    if (it->callDepth + 1 > it->maxCallDepth) {
        error(
            it, callExpr->op,
            StrFormat("Maximum call depth reached : %zu", it->callDepth)
        );
        return MakeNil();
    }

    it->callDepth++;

    PEnv *fnEnv = NewEnv(it->gc, (PEnv *)f->env);
    for (u64 i = 0; i < count; i++) {
        EnvPutValue(fnEnv, f->params[i]->hash, args[i]);
    }

    ExResult evalOut = execBlock(it, f->body, fnEnv, true);

    if (it->callDepth >= 0) {
        it->callDepth--;
    }

    if (evalOut.type == ET_RETURN) {
        RecycleEnv(it->gc, fnEnv);
        return evalOut.value;
    }

    RecycleEnv(it->gc, fnEnv);

    return evalOut.value;
}

// Handle User-Defined Function Calls
static PValue callFunction(
    PInterpreter *it, PObj *func, PExpr *callExpr, PEnv *env
) {
    assert(func->type == OT_FNC);
    assert(callExpr->type == EXPR_CALL);

    struct OFunction *funcObj = &func->v.OFunction;
    struct ECall *call = &callExpr->exp.ECall;

    if (call->argCount != funcObj->paramCount) {
        error(
            it, callExpr->op,
            StrFormat(
                "Function needs %zu arguments but %zu was given when calling",
                funcObj->paramCount, call->argCount
            )
        );
        return MakeNil();
    }

    // We are making the arguments stack to hold 8 values
    // I don't think, There will be many use cases where more than 8 will be
    // needed
    PValue argStack[8];
    PValue *argPtr = NULL;
    bool argsOnHeap = false;

    if (call->argCount > 8) {
        argPtr = PCalloc(call->argCount, sizeof(PValue));
        if (argPtr == NULL) {
            error(
                it, call->op,
                "Internal Error : Failed to Allocate memory for call "
                "arguments"
            );
            return MakeNil();
        }
        argsOnHeap = true;
    } else {
        argPtr = argStack;
        argsOnHeap = false;
    }

    for (u64 i = 0; i < call->argCount; i++) {
        argPtr[i] = evaluate(it, call->args[i], env);
    }

    PValue value = handleCall(it, func, argPtr, call->argCount, callExpr);
    if (argsOnHeap) {
        PFree(argPtr);
    }
    if (IsValueError(value)) {
        error(it, callExpr->op, GetErrorObjMsg(ValueAsObj(value)));
    }
    return value;
}

// Handle Native Function Calls
static PValue callNative(
    PInterpreter *it, PObj *nfunc, PExpr *callExpr, PEnv *env
) {
    assert(nfunc->type == OT_NATIVE);
    assert(callExpr->type == EXPR_CALL);

    struct ONative *nfuncObj = &nfunc->v.ONative;
    struct ECall *call = &callExpr->exp.ECall;

    if (nfuncObj->arity >= 0 && call->argCount != nfuncObj->arity) {
        error(
            it, callExpr->op,
            StrFormat(
                "Function needs %d arguments but %d was given when calling",
                nfuncObj->arity, call->argCount
            )
        );
        return MakeNil();
    }

    PValue argStack[8];
    PValue *argPtr = NULL;
    bool argsOnHeap = false;
    if (call->argCount > 8) {
        argPtr = PCalloc(call->argCount, sizeof(PValue));
        if (argPtr == NULL) {
            error(
                it, call->op,
                "Internal Error : Failed to Allocate memory for call "
                "arguments"
            );
            return MakeNil();
        }
        argsOnHeap = true;
    } else {
        argPtr = argStack;
        argsOnHeap = false;
    }

    for (u64 i = 0; i < call->argCount; i++) {
        argPtr[i] = evaluate(it, call->args[i], env);
    }

    PValue value = nfuncObj->fn(it, argPtr, call->argCount);

    if (argsOnHeap) {
        PFree(argPtr);
    }

    if (IsValueError(value)) {
        error(it, callExpr->op, GetErrorObjMsg(ValueAsObj(value)));
    }

    return value;
}

// Function Call Operation
static PValue vCall(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_CALL);
    struct ECall *ec = &expr->exp.ECall;
    PValue co = evaluate(it, ec->callee, env);
    if (!IsValueObj(co)) {
        error(it, ec->callee->op, "Can only call functions");
        return MakeNil();
    }

    if (IsValueObj(co)) {
        PObj *callObj = ValueAsObj(co);
        if (IsValueObjType(co, OT_FNC)) {
            return callFunction(it, callObj, expr, env);
        } else if (IsValueObjType(co, OT_NATIVE)) {
            return callNative(it, callObj, expr, env);
        }
    }

    error(it, ec->callee->op, "Only functions can be called");
    return MakeNil();
}

// Create New Array Literal Object
static PValue vArray(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_ARRAY);
    struct EArray *arr = &expr->exp.EArray;
    PValue *vitems = NULL;

    for (u64 i = 0; i < arr->count; i++) {
        arrput(vitems, evaluate(it, arr->items[i], env));
    }

    PObj *arrObj = NewArrayObject(it->gc, arr->op, vitems, arr->count);
    return MakeObject(arrObj);
}

// Create New Map Literal Object
static PValue vMap(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_MAP);
    struct EMap *map = &expr->exp.EMap;

    MapEntry *table = NULL;
    MapEntry s;

    for (u64 i = 0; i < map->count; i += 2) {
        PValue k = evaluate(it, map->etable[i], env);
        u64 keyHash = GetValueHash(k, (u64)it->gc->timestamp);
        if (!CanValueBeKey(k)) {
            if (table != NULL) {
                hmfree(table);
            }
            error(it, map->etable[i]->op, "Invalid key for map");
            return MakeNil();
        }
        PValue v = evaluate(it, map->etable[i + 1], env);
        s = (MapEntry){keyHash, k, v};
        hmputs(table, s);
    }

    PObj *mapObj = NewMapObject(it->gc, map->op);
    mapObj->v.OMap.table = table;
    mapObj->v.OMap.count = (u64)hmlen(table);
    return MakeObject(mapObj);
}

// Subscript for Arrays
static PValue arraySubscript(
    PInterpreter *it, PObj *arrObj, PExpr *expr, PEnv *env
) {
    assert(arrObj->type == OT_ARR);
    assert(expr->type == EXPR_SUBSCRIPT);

    struct ESubscript *sub = &expr->exp.ESubscript;
    PValue indexValue = evaluate(it, sub->index, env);
    if (!IsValueNum(indexValue)) {
        error(it, sub->index->op, "Arrays can only indexed with integers");
        return MakeNil();
    }

    double indexDbl = ValueAsNum(indexValue);

    if (!IsDoubleInt(indexDbl)) {
        error(it, sub->index->op, "Arrays can only indexed with integers");
        return MakeNil();
    }

    int indexInt = (int)indexDbl;
    struct OArray *array = &arrObj->v.OArray;

    if (indexInt >= array->count) {
        error(it, sub->index->op, "Array Index out of range");
        return MakeNil();
    }
    return array->items[indexInt];
}

// subscripting for Map
static PValue mapSubscript(
    PInterpreter *it, PObj *mapObj, PExpr *expr, PEnv *env
) {
    assert(mapObj->type == OT_MAP);
    assert(expr->type == EXPR_SUBSCRIPT);

    struct ESubscript *sub = &expr->exp.ESubscript;
    PValue keyValue = evaluate(it, sub->index, env);

    if (!CanValueBeKey(keyValue)) {
        error(it, sub->index->op, "Invalid key value for map subscripting");
        return MakeNil();
    }

    struct OMap *map = &mapObj->v.OMap;
    u64 keyHash = GetValueHash(keyValue, it->gc->timestamp);

    if (hmgeti(map->table, keyHash) != -1) {
        return hmgets(map->table, keyHash).value;
    } else {
        error(it, sub->index->op, "Key doesn't exist for map");
        return MakeNil();
    }
}

// Evaluate subscripting expression
static PValue vSubscript(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_SUBSCRIPT);
    struct ESubscript *sub = &expr->exp.ESubscript;
    PValue subValue = evaluate(it, sub->value, env);

    if (IsValueObj(subValue)) {
        PObj *subObj = ValueAsObj(subValue);
        if (subObj->type == OT_ARR) {
            return arraySubscript(it, subObj, expr, env);
        } else if (subObj->type == OT_MAP) {
            return mapSubscript(it, subObj, expr, env);
        }
    }

    error(
        it, sub->op, "Invalid Subscript. Subscript only works on array and maps"
    );
    return MakeNil();
}

static PValue vModget(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_MODGET);
    struct EModget *mg = &expr->exp.EModget;

    if (mg->module->type != EXPR_VARIABLE) {
        error(it, expr->op, "Module in not a name");
    }

    // The module token ->math<-.pow(..)
    Token *modTok = mg->module->exp.EVariable.name;
    // The child token math.->pow<-(..)
    Token *childTok = mg->child;

    if (hmgeti(it->proxyTable, modTok->hash) != -1) {
        PModule *mod = hmgets(it->proxyTable, modTok->hash).mod;
        bool childFound = true;
        PValue childValue = EnvGetValue(mod->env, childTok->hash, &childFound);
        if (!childFound) {
            error(it, childTok, "Child not found for module");
            return MakeNil();
        }
        return childValue;
    } else {
        error(it, modTok, "Module not found");
        return MakeNil();
    }
    return MakeNil();
}

static PValue evaluate(PInterpreter *it, PExpr *expr, PEnv *env) {
    if (expr == NULL) {
        return MakeNil();
    }
    if (env == NULL) {
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
        case EXPR_MAP: return vMap(it, expr, env);
        case EXPR_SUBSCRIPT: return vSubscript(it, expr, env);
        case EXPR_MODGET: return vModget(it, expr, env);
    }

    return MakeNil();
}

static ExResult execBlock(PInterpreter *it, PStmt *stmt, PEnv *env, bool ret) {
    PStmt **stmtList = stmt->stmt.SBlock.stmts;

    if (stmtList == NULL) {
        return ExSimple(MakeNil());
    }

    ExResult temp = ExSimple(MakeNil());
    PEnv *blockEnv = NewEnv(it->gc, env);

    for (int i = 0; i < arrlen(stmtList); i++) {
        temp = execute(it, stmtList[i], blockEnv);
        if (ret && temp.type == ET_RETURN) {
            RecycleEnv(it->gc, blockEnv);
            return temp;
        }
        if (temp.type == ET_BREAK) {
            RecycleEnv(it->gc, blockEnv);
            return temp;
        }
    }

    RecycleEnv(it->gc, blockEnv);
    return temp;
}

// Execute block statements
static ExResult vsBlock(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_BLOCK);
    return execBlock(it, stmt, env, true);
}

// Execute variable declaration statements
static ExResult vsLet(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_LET);
    PValue value = evaluate(it, stmt->stmt.SLet.expr, env);
    EnvPutValue(env, stmt->stmt.SLet.name->hash, value);
    return ExSimple(value);
}

// Temporary print statements
static ExResult vsPrint(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_PRINT);
    PValue value = evaluate(it, stmt->stmt.SPrint.value, env);
    PrintValue(value);
    printf("\n");
    return ExSimple(MakeNil());
}

// Execute Expression statements
static ExResult vsExprStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_EXPR);
    return ExSimple(evaluate(it, stmt->stmt.SExpr.expr, env));
}

// Execute If-Else conditional statements
static ExResult vsIfStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_IF);
    PValue cond = evaluate(it, stmt->stmt.SIf.cond, env);
    if (IsValueTruthy(cond)) {
        return execute(it, stmt->stmt.SIf.thenBranch, env);
    } else {
        if (stmt->stmt.SIf.elseBranch != NULL) {
            return execute(it, stmt->stmt.SIf.elseBranch, env);
        }
    }

    return ExSimple(MakeNil());
}

// Execute While-Do loop statements
static ExResult vsWhileStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_WHILE);
    PValue cond = evaluate(it, stmt->stmt.SWhile.cond, env);
    while (IsValueTruthy(cond)) {
        ExResult res = execute(it, stmt->stmt.SWhile.body, env);
        if (res.type == ET_BREAK) {
            break;
        } else if (res.type == ET_RETURN) {
            return res;
        }

        cond = evaluate(it, stmt->stmt.SWhile.cond, env);
    }

    return ExSimple(MakeNil());
}

// Execute return statements
static ExResult vsReturnStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_RETURN);

    struct SReturn *ret = &stmt->stmt.SReturn;
    PValue value = evaluate(it, ret->value, env);
    return ExReturn(value);
}

// Execute break statements
static ExResult vsBreakStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_BREAK);
    return ExBreak();
}

// Execute function declaration statements
static ExResult vsFuncStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_FUNC);
    struct SFunc *fs = &stmt->stmt.SFunc;
    PEnv *closureEnv = NewEnv(it->gc, NULL);
    PObj *f = NewFuncObject(
        it->gc, fs->name, fs->params, fs->body, closureEnv, fs->paramCount
    );

    EnvPutValue(env, fs->name->hash, MakeObject(f));
    EnvCaptureUpvalues(it->gc, env, closureEnv);
    return ExSimple(MakeNil());
}

static ExResult vsImportStmt(PInterpreter *it, PStmt *stmt, PEnv *env) {
    assert(stmt->type == STMT_IMPORT);
    struct SImport *imp = &stmt->stmt.SImport;

    PValue pathValue = evaluate(it, imp->path, env);

    if (!IsValueObjType(pathValue, OT_STR)) {
        error(it, imp->op, "Import path must be a string");
        return ExSimple(MakeNil());
    }

    char *pathStr = ValueAsObj(pathValue)->v.OString.value;
    PModule *mod = NewModule(it, pathStr);
    if (mod == NULL) {
        error(it, stmt->op, "Failed to create module");
        return ExSimple(MakeNil());
    }
    PushModule(it, mod);
    u64 key = imp->name->hash;
    PushProxy(it, key, imp->name->lexeme, mod);
    StdlibMod stdmod = GetStdlibMod(mod->pathname);
    if (stdmod != STDLIB_NONE) {
        PushStdlib(it, mod->env, mod->pathname, stdmod);
    } else {
        error(
            it, imp->op,
            "Stdlib module not found. Currently stdlib modules are supported "
            "only"
        );
    }
    return ExSimple(MakeNil());
}

static ExResult execute(PInterpreter *it, PStmt *stmt, PEnv *env) {
    if (stmt == NULL) {
        return ExSimple(MakeNil());
    }

    if (env == NULL) {
        return ExSimple(MakeNil());
    }

    switch (stmt->type) {
        case STMT_PRINT: return vsPrint(it, stmt, env);
        case STMT_EXPR: return vsExprStmt(it, stmt, env);
        case STMT_LET: return vsLet(it, stmt, env);
        case STMT_BLOCK: return vsBlock(it, stmt, env);
        case STMT_IF: return vsIfStmt(it, stmt, env);
        case STMT_WHILE: return vsWhileStmt(it, stmt, env);
        case STMT_RETURN: return vsReturnStmt(it, stmt, env);
        case STMT_BREAK: return vsBreakStmt(it, stmt, env);
        case STMT_FUNC: return vsFuncStmt(it, stmt, env);
        case STMT_IMPORT: return vsImportStmt(it, stmt, env);
        default: error(it, NULL, "Unknown statement found!");
    }

    return ExSimple(MakeNil());
}
