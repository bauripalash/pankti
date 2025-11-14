#include "include/interpreter.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/native.h"
#include "include/object.h"
#include "include/pstdlib.h"
#include "include/token.h"
#include "include/utils.h"
#include <stddef.h>
#include <string.h>

#ifdef PANKTI_BUILD_DEBUG
#undef NDDEBUG
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

// For Debug: Get ExResult Type as String
static char *ExResultToStr(ExType t) {
    switch (t) {
        case ET_SIMPLE: return "Simple";
        case ET_BREAK: return "Break";
        case ET_RETURN: return "return";
    }
}

// Create a Simple Execution Result
static inline ExResult ExSimple(PValue v) {
    ExResult er;
    er.type = ET_SIMPLE;
    er.value = v;
    return er;
}

// Create a Break Execution Result
static inline ExResult ExBreak() {
    ExResult er;
    er.type = ET_BREAK;
    er.value = MakeNil();
    return er;
}

// Create a Return Execution Result
static inline ExResult ExReturn(PValue v) {
    ExResult er;
    er.type = ET_RETURN;
    er.value = v;
    return er;
}

static PModule *NewModule(PInterpreter *it, char * name){
	PModule * mod = PMalloc(sizeof(PModule));
	if (mod == NULL) {
		//throw error
		return NULL;
	}
	mod->pathname = StrDuplicate(name, strlen(name));
	// error check
	mod->env = NewEnv(NULL);
	return mod;
}

static void PushModule(PInterpreter *it, PModule * mod){
	arrput(it->mods, mod);
	it->modCount = arrlen(it->mods);
}

static void PushProxy(PInterpreter *it, uint64_t key, char * name, PModule *mod){
	if (mod == NULL) {
		return;//error check
	}

	ModProxyEntry s = {.key = key, .name = name, .mod = mod};
	hmputs(it->proxyTable, s);
	it->proxyCount = hmlen(it->proxyTable);
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
    it->env = NewEnv(NULL);
    it->gc = gc;
	it->proxyTable = NULL;
	it->proxyCount = 0;
	it->mods = NULL;
	it->modCount = 0;
    RegisterNatives(it, it->env);
    return it;
}
void FreeInterpreter(PInterpreter *it) {
    if (it == NULL) {
        return;
    }

    if (it->env != NULL) {
        FreeEnv(it->env);
    }

	if (it->proxyCount > 0 && it->proxyTable != NULL) {
		hmfree(it->proxyTable);
	}

	if (it->modCount > 0 && it->mods != NULL) {
		for (int i = 0; i < it->modCount; i++) {
			PModule * mod = arrpop(it->mods);
			PFree(mod->pathname);
			FreeEnv(mod->env);
			PFree(mod);
		}

		arrfree(it->mods);
	}

    PFree(it);
}
void Interpret(PInterpreter *it) {
    for (int i = 0; i < arrlen(it->program); i++) {
        execute(it, it->program[i], it->env);
    }

    // FreeObject(it->gc, obj);
}

// Report a Error
static void error(PInterpreter *it, Token *tok, const char *msg) {
    CoreError(it->core, tok, msg);
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
            if (l.type == VT_NUM && r.type == VT_NUM) {
                double value = l.v.num + r.v.num;
                return MakeNumber(value);
            } else {
                if ((l.type == VT_OBJ && ValueAsObj(l)->type == OT_STR) &&
                    (r.type == VT_OBJ && ValueAsObj(r)->type == OT_STR)) {
                    bool ok = true;
                    struct OString *ls = &ValueAsObj(l)->v.OString;
                    size_t lsLen = strlen(ls->value);
                    struct OString *rs = &ValueAsObj(r)->v.OString;
                    size_t rsLen = strlen(rs->value);
                    char *newStr =
                        StrJoin(ls->value, lsLen, rs->value, rsLen, &ok);
                    if (!ok) {
                        error(it, expr->op, "Fail to join string");
                        return MakeNil();
                    }

                    PObj *nsObj = NewStrObject(it->gc, expr->op, newStr, true);
                    return MakeObject(nsObj);
                }
            }
            break;
        }
            // Multiplication
        case T_ASTR: {
            if (l.type != VT_NUM || r.type != VT_NUM) {
                error(
                    it, expr->op, "Multiplication can only be done with numbers"
                );
                return MakeNil();
            }
            double value = l.v.num * r.v.num;
            return MakeNumber(value);
            break;
        }
        case T_MINUS: {
            if (l.type != VT_NUM || r.type != VT_NUM) {
                error(
                    it, expr->op, "Substraction can only be done with numbers"
                );
                return MakeNil();
            }
            double value = l.v.num - r.v.num;
            return MakeNumber(value);
            break;
        }
            // Division
        case T_SLASH: {
            if (l.type != VT_NUM || r.type != VT_NUM) {
                error(it, expr->op, "Division can only be done with numbers");
                return MakeNil();
            }

            if (r.v.num == 0) {
                error(it, expr->op, "Division by zero");
                return MakeNil();
            }

            double value = l.v.num / r.v.num;
            return MakeNumber(value);
            break;
        }
        case T_EQEQ: return MakeBool(IsValueEqual(&l, &r));
        case T_BANG_EQ: return MakeBool(!IsValueEqual(&l, &r));
        case T_GT: return MakeBool(l.v.num > r.v.num);
        case T_GTE: return MakeBool(l.v.num >= r.v.num);
        case T_LT: return MakeBool(l.v.num < r.v.num);
        case T_LTE: return MakeBool(l.v.num <= r.v.num);
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
    if (indexValue.type != VT_NUM) {
        error(it, index->op, "Array Objects can be indexed only with Integers");
        return;
    }

    double indexDouble = indexValue.v.num;

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

    if (!CanValueBeKey(&keyValue)) {
        error(it, keyExpr->op, "Invalid key for map");
        return;
    }

    uint64_t keyHash = GetValueHash(&keyValue, (uint64_t)it->gc->timestamp);
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
    if (coreValue.type == VT_OBJ) {
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
            double value = -r.v.num;
            return MakeNumber(value);
        }
        case T_BANG: {
            return MakeBool(!IsValueTruthy(&r));
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
        if (IsValueTruthy(&left)) {
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

    error(it, expr->op, "Invalid logical operation");
    return MakeNil();
}

// Evaluate a function call and return result (if any)
static PValue handleCall(
    PInterpreter *it, PObj *func, PValue *args, int count
) {
    assert(func->type == OT_FNC);
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
                "Function needs %d arguments but %d was given when calling",
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

    if (call->argCount > 8) {
        argPtr = PCalloc(8, sizeof(PValue));
        if (argPtr == NULL) {
            error(
                it, call->op,
                "Internal Error : Failed to Allocate memory for call "
                "arguments"
            );
            return MakeNil();
        }
    } else {
        argPtr = argStack;
    }

    for (int i = 0; i < call->argCount; i++) {
        argPtr[i] = evaluate(it, call->args[i], env);
    }

    PValue value = handleCall(it, func, argPtr, call->argCount);
    if (call->argCount > 16) {
        PFree(argPtr);
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
    if (call->argCount > 8) {
        argPtr = PCalloc(8, sizeof(PValue));
        if (argPtr == NULL) {
            error(
                it, call->op,
                "Internal Error : Failed to Allocate memory for call "
                "arguments"
            );
            return MakeNil();
        }
    } else {
        argPtr = argStack;
    }

    for (int i = 0; i < call->argCount; i++) {
        argPtr[i] = evaluate(it, call->args[i], env);
    }

    PValue value = nfuncObj->fn(it, argPtr, call->argCount);

    if (call->argCount > 8) {
        PFree(argPtr);
    }

    return value;
}

// Function Call Operation
static PValue vCall(PInterpreter *it, PExpr *expr, PEnv *env) {
    assert(expr->type == EXPR_CALL);
    struct ECall *ec = &expr->exp.ECall;
    PValue co = evaluate(it, ec->callee, env);
    if (co.type != VT_OBJ) {
        error(it, ec->callee->op, "Can only call functions");
        return MakeNil();
    }

    if (co.type == VT_OBJ) {
        PObj *callObj = ValueAsObj(co);
        if (co.v.obj->type == OT_FNC) {
            return callFunction(it, callObj, expr, env);
        } else if (co.v.obj->type == OT_NATIVE) {
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

    for (int i = 0; i < arr->count; i++) {
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

    for (int i = 0; i < map->count; i += 2) {
        PValue k = evaluate(it, map->etable[i], env);
        uint64_t keyHash = GetValueHash(&k, (uint64_t)it->gc->timestamp);
        if (!CanValueBeKey(&k)) {
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
    mapObj->v.OMap.count = hmlen(table);
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
    if (indexValue.type != VT_NUM) {
        error(it, sub->index->op, "Arrays can only indexed with integers");
        return MakeNil();
    }

    double indexDbl = indexValue.v.num;

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

    if (!CanValueBeKey(&keyValue)) {
        error(it, sub->index->op, "Invalid key value for map subscripting");
        return MakeNil();
    }

    struct OMap *map = &mapObj->v.OMap;
    uint64_t keyHash = GetValueHash(&keyValue, it->gc->timestamp);

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

    if (subValue.type == VT_OBJ) {
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

static PValue vModget(PInterpreter * it, PExpr * expr, PEnv * env){
	assert(expr->type == EXPR_MODGET);
	struct EModget * mg = &expr->exp.EModget;
	
	if (mg->module->type != EXPR_VARIABLE) {
		error(it, expr->op, "Module in not a name");
	}

	// The module token ->math<-.pow(..)
	Token * modTok = mg->module->exp.EVariable.name;
	// The child token math.->pow<-(..)
	Token * childTok = mg->child;

	if (hmgeti(it->proxyTable, modTok->hash) != -1) {
		PModule * mod = hmgets(it->proxyTable, modTok->hash).mod;
		bool childFound = true;
		PValue childValue = EnvGetValue(mod->env, childTok->hash, &childFound);
		if (!childFound) {
			error(it, childTok, "Child not found for module");
			return MakeNil();
		}
		return childValue;
	} else{
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

    ExResult temp;
    PEnv *blockEnv = NewEnv(env);

    for (int i = 0; i < arrlen(stmtList); i++) {
        temp = execute(it, stmtList[i], blockEnv);
        if (ret && temp.type == ET_RETURN) {
            FreeEnv(blockEnv);
            return temp;
        }
        if (temp.type == ET_BREAK) {
            FreeEnv(blockEnv);
            return temp;
        }
    }

    FreeEnv(blockEnv);
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
    PrintValue(&value);
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
    if (IsValueTruthy(&cond)) {
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
    while (IsValueTruthy(&cond)) {
        ExResult res = execute(it, stmt->stmt.SWhile.body, env);
        if (res.type == ET_BREAK) {
            break;
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
    PObj *f = NewFuncObject(
        it->gc, fs->name, fs->params, fs->body, NewEnv(env), fs->paramCount
    );

    EnvPutValue(env, fs->name->hash, MakeObject(f));
    return ExSimple(MakeNil());
}

static ExResult vsImportStmt(PInterpreter * it, PStmt *stmt, PEnv * env){
	assert(stmt->type == STMT_IMPORT);
	struct SImport * imp = &stmt->stmt.SImport;
	
	PValue pathValue = evaluate(it, imp->path, env);

	if (!IsValueObjType(&pathValue, OT_STR)) {
		error(it, imp->op, "Import path must be a string");
		return ExSimple(MakeNil());
	}

	char * pathStr = pathValue.v.obj->v.OString.value;
	PModule * mod = NewModule(it, pathStr);
	if (mod == NULL) {
		error(it, stmt->op, "Failed to create module");
		return ExSimple(MakeNil());
	}
	PushModule(it, mod);
	uint64_t key = imp->name->hash;
	PushProxy(it, key, imp->name->lexeme, mod);
	StdlibMod stdmod = GetStdlibMod(mod->pathname);
	if (stdmod != STDLIB_NONE) {
		PushStdlib(it, mod->env, mod->pathname, stdmod);
	} else{
		error(it, imp->op, "Stdlib module not found. Currently stdlib modules are supported only");
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
        case STMT_PRINT: return vsPrint(it, stmt, env); break;
        case STMT_EXPR: return vsExprStmt(it, stmt, env); break;
        case STMT_LET: return vsLet(it, stmt, env); break;
        case STMT_BLOCK: return vsBlock(it, stmt, env); break;
        case STMT_IF: return vsIfStmt(it, stmt, env); break;
        case STMT_WHILE: return vsWhileStmt(it, stmt, env); break;
        case STMT_RETURN: return vsReturnStmt(it, stmt, env); break;
        case STMT_BREAK: return vsBreakStmt(it, stmt, env);
        case STMT_FUNC: return vsFuncStmt(it, stmt, env);
		case STMT_IMPORT: return vsImportStmt(it, stmt, env);
        default: error(it, NULL, "Unknown statement found!");
    }

    return ExSimple(MakeNil());
}
