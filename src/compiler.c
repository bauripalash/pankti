#include "include/compiler.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/compiler_errors.h"
#include "include/flags.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/strescape.h"
#include "include/token.h"
#include "include/utils.h"
#include "include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_CONST_COUNT 65535

// Compile and Emit Bytecodes for a Parser Produced Statement
static bool compileStmt(PCompiler *comp, PStmt *stmt);
// Compiled and Emit Bytecodes for a Parser Produced Expression
static bool compileExpr(PCompiler *comp, PExpr *expr);
// Add a Constant and return its index in constant list
static u16 addConstant(PCompiler *comp, PValue value);
// Try setting up variable name.
// If is local, we create a local
// otherwise we create a Identifier constant from token
static u16 readVariableName(PCompiler *comp, Token *name);

PCompiler *dummyCompiler(
    Pgc *gc, PCompiler *enclosing, PCompFuncType ftype, Token *name
) {
    PCompiler *c = PCreate(PCompiler);
    if (c == NULL) {
        return NULL;
    }
    c->gc = gc;
    c->func = NULL;
    c->funcType = ftype;
    c->enclosing = enclosing;
    if (enclosing != NULL) {
        c->errCtx = enclosing->errCtx;
    }
    c->prog = NULL;
    c->progCount = 0;
    c->scopeDepth = 0;
    c->localCount = 0;
    c->dummyToken = NewToken(T_EOF);
    if (c->dummyToken == NULL) {
        PFree(c);
        return NULL;
    }

    PObj *cmFunc = NULL;
    if (name != NULL && ftype == COMP_FN_FUNCTION) {
        cmFunc = NewComFuncObject(c->gc, name);

    } else {
        cmFunc = NewComFuncObject(c->gc, NULL);
    }
    if (cmFunc == NULL) {
        FreeToken(c->dummyToken);
        PFree(c);
        return NULL;
    }
    c->func = cmFunc;

    c->loopCtx = NULL;

    PLocal *local = &c->locals[c->localCount++];
    local->depth = 0;
    local->name = NULL;
    local->isCaptured = false;
    return c;
}

PCompiler *NewCompiler(Pgc *gc) {
    PCompiler *c = dummyCompiler(gc, NULL, COMP_FN_SCRIPT, NULL);
    if (c == NULL) {
        return NULL;
    }

    return c;
}

PCompiler *NewEnclosedCompiler(
    Pgc *gc, PCompiler *comp, PCompFuncType ftype, Token *name
) {

    PCompiler *c = dummyCompiler(gc, comp, COMP_FN_FUNCTION, name);
    if (c == NULL) {
        return NULL;
    }

    return c;
}

void FreeCompiler(PCompiler *comp) {
    if (comp == NULL) {
        return;
    }
    if (comp->loopCtx != NULL) {
        PFree(comp->loopCtx);
    }
    FreeToken(comp->dummyToken);
    PFree(comp);
}

void DebugCompilerTree(PCompiler *comp) {
    if (comp == NULL || comp->gc == NULL) {
        return;
    }
    PanPrint("---->\n");
    PCompiler *current = comp;
    int i = 0;
    while (current != NULL) {
        for (int j = 0; j <= i; j++) {
            PanPrint(" ");
        }
        PrintObject(current->func);
        PanPrint("\n");
        i += 1;
        current = current->enclosing;
    }

    PanPrint("<----\n");
}

void CompilerMarkRoots(Pgc *gc, void *ctx) {
    PCompiler *comp = (PCompiler *)ctx;
    if (comp == NULL || comp->gc == NULL) {
        return;
    }

    PCompiler *current = comp;
    while (current != NULL) {
        GcMarkObject(comp->gc, current->func);
        current = current->enclosing;
    }
}

static void cmpError(PCompiler *comp, Token *token, const char *msg) {
    comp->errCtx.report(comp->errCtx.ctx, token, msg, true);
}

// Get Current Compiling Bytecode object
static finline PBytecode *getbt(PCompiler *comp) {
    return comp->func->v.OComFunction.code;
}

// Add a Constant and return its index in constant list
static u16 addConstant(PCompiler *comp, PValue value) {
    u16 index = AddConstantToPool(getbt(comp), value);
    return index;
}

// Emit a single opcode
static u64 emitBt(PCompiler *comp, Token *tok, PanOpCode op) {
    return EmitBytecode(getbt(comp), tok, op);
}

// Emit a bytecode with a u16 operand
static u64 emitBtU16(PCompiler *comp, Token *tok, PanOpCode op, u16 a) {
    return EmitBytecodeWithOneArg(getbt(comp), tok, op, a);
}

// Emit a jump type opcode with placeholder which should be patched later
// returns the position of offset operand
static u16 emitJump(PCompiler *comp, Token *tok, PanOpCode op) {
    emitBtU16(comp, tok, op, 0xffff);
    return getbt(comp)->codeCount - 2;
}

// Patch a jump type opcode's operand
// offset is the position of offset operand in bytecode
static void patchJump(PCompiler *comp, int offset) {
    int jump = getbt(comp)->codeCount - offset - 2;
    if (jump > UINT16_MAX) {
        cmpError(comp, NULL, CMP_ERR_JUMP_TOO_BIG);
        return;
    }

    getbt(comp)->code[offset] = (jump >> 8) & 0xff;
    getbt(comp)->code[offset + 1] = jump & 0xff;
}

// Start a scope
// Increase a scope depth
static int startScope(PCompiler *comp) {
    comp->scopeDepth++;
    return comp->scopeDepth;
}

// End a scope
// Pop all the local variables
static int endScope(PCompiler *comp, Token *op) {
    comp->scopeDepth--;

    while (comp->localCount > 0 &&
           comp->locals[comp->localCount - 1].depth > comp->scopeDepth) {

        // if the local variable is used as upvalue for inner scopes
        // close the upvalue, otherwise just pop it from stack
        if (comp->locals[comp->localCount - 1].isCaptured) {
            emitBt(comp, op, OP_CLS_UPVAL);
        } else {
            emitBt(comp, op, OP_POP);
        }
        comp->localCount--;
    }

    return comp->scopeDepth;
}

// Check if token 'a' and 'b' has same lexeme
static bool isIdentTokenEqual(Token *a, Token *b) {
    if (a->len != b->len) {
        return false;
    }

    return memcmp(a->lexeme, b->lexeme, a->len) == 0;
}

// Find if a local with name exists
// Searches all locals. Return the index if found
// otherwise returns `-1`
static int findLocal(PCompiler *comp, Token *name) {
    for (int i = comp->localCount - 1; i >= 0; i--) {
        PLocal *lcl = &comp->locals[i];

        if (lcl->name != NULL && isIdentTokenEqual(name, lcl->name)) {
            if (lcl->depth == -1) {
                cmpError(comp, name, CMP_ERR_VAR_OWN_INIT);
                return -2;
            }
            return i;
        }
    }
    return -1;
}

// Mark latest declared local variable as usable
static void markLocalInit(PCompiler *comp) {
    if (comp->scopeDepth == 0) {
        return;
    }
    comp->locals[comp->localCount - 1].depth = comp->scopeDepth;
}

// Add upvalue to compiler
static int addUpvalue(PCompiler *comp, u16 index, bool isLocal) {
    // Current compiling function
    struct OComFunction *compFun = &comp->func->v.OComFunction;
    i16 upvalCount = compFun->upvalCount;

    // check if same upvalue has already been made
    for (i16 i = 0; i < upvalCount; i++) {
        UpValue *upval = &comp->upvals[i];
        if (upval->index == index && upval->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalCount == UINT8_MAX) {
        cmpError(
            comp, compFun->rawName,
            "Internal Error : Too many closure variables in function"
        );
        return 0;
    }

    comp->upvals[upvalCount].isLocal = isLocal;
    comp->upvals[upvalCount].index = index;
    return compFun->upvalCount++;
}

// Find upvalue in local context and enclosing context
static int findUpvalue(PCompiler *comp, Token *name) {
    if (comp->enclosing == NULL) { // no outer scope means no local or upvalue
        return -1;
    }

    // Look for local variables defined in just outside the the scope
    // func mango()
    //     let x = 10
    //     func apple()
    //         ?x // <-- here x value would be just outside the scope of apple
    //            // thats where the findLocal(...) succeed.
    //            // as `mango` is enclosing compiler for `apple`
    //            // we flag the found local as isCaptured
    //            // to tell the interpreter that is used by inner functions
    //     end
    // end
    int local = findLocal(comp->enclosing, name);
    if (local >= 0) {
        comp->enclosing->locals[local].isCaptured = true;
        return addUpvalue(comp, (u16)local, true);
    }

    // Look for local variables defined in just outside the the scope
    // func mango()
    //     let x = 10
    //     func apple()
    //         func banana()
    //             ?x   // <-- x is not defined just outside banana
    //                  // but it is just outside banana's enclosing compiler
    //                  // so it is in compiler->enclosing->enclosing
    //                  // that means x is upvalue in context of apple
    //                  // so x is upvalue of upvalue
    //                  // in worst case ->
    //                  // we will look for `x` till the root script
    //                  // in apple scope, above `findLocal` will return
    //                  // stack index, and flag it `isCaptured`
    //         end
    //     end
    // end

    int upval = findUpvalue(comp->enclosing, name);
    if (upval >= 0) {
        return addUpvalue(comp, (u16)upval, false);
    }

    // we couldn't find any local or upvalue, so our dumb compiler assumes
    // it must be a global variables, because we don't throw errors for
    // non-existing variables till VM executation
    return -1;
}

// Emit return opcode
static finline void emitReturn(PCompiler *comp) {
    // EmitRawU8(getbt(comp), OP_RETURN);
    emitBt(comp, comp->dummyToken, OP_NIL);
    emitBt(comp, comp->dummyToken, OP_RETURN);
}

// Time has come for ending of compiler
// Emit return and return current compiling function
static finline PObj *endCompiler(PCompiler *comp) {
    emitReturn(comp);
    return comp->func;
}

// Return current compiling function
// If the compiling should end, must call endCompiler before this function
PObj *GetCompiledFunction(PCompiler *comp) {
    if (comp == NULL || comp->func == NULL) {
        return NULL;
    }

    return comp->func;
}

bool CompilerCompile(PCompiler *compiler, PStmt **prog) {
    compiler->prog = prog;
    compiler->progCount = (u64)arrlen(prog);

    for (u64 i = 0; i < compiler->progCount; i++) {
        if (!compileStmt(compiler, prog[i])) {
            cmpError(compiler, prog[i]->op, CMP_ERR_FAIL_TOP_LVL_STMT);
            return false;
        }
    }
    endCompiler(compiler);

    return true;
}

static char *readStringEscapes(PCompiler *comp, Token *tok) {
    char *rawinput = tok->lexeme;
    u64 inlen = (u64)strlen(rawinput);
    u64 outlen = inlen * 4 + 1;
    char *output = PCalloc((u64)outlen, sizeof(char));
    if (output == NULL) {
        return NULL;
    }

    StrEscapeErr err = ProcessStringEscape(rawinput, inlen, output, outlen);
    switch (err) {
        case SESC_OK: break;
        case SESC_UNKNOWN_ESCAPE: {
            cmpError(comp, tok, CMP_ERR_SESC_UNKN_ESC);
            break;
        }
        case SESC_INVALID_HEX_CHAR: {
            cmpError(comp, tok, CMP_ERR_SESC_INVLD_HEX);
            break;
        }

        case SESC_BUFFER_NOT_ENOUGH: {
            cmpError(comp, tok, CMP_ERR_SESC_BFR_NOT_ENGH);
            break;
        }
        case SESC_INPUT_FINISHED_EARLY: {
            cmpError(comp, tok, CMP_ERR_SESC_FNSH_ERLY);
            break;
        }

        case SESC_NO_LOW_SURROGATE: {
            cmpError(comp, tok, CMP_ERR_SESC_NO_LO_SRGT);
            break;
        }
        case SESC_LONE_LOW_SURROGATE: {
            cmpError(comp, tok, CMP_ERR_SESC_LN_LOW_SURROGATE);
            break;
        }

        case SESC_INVALID_LOW_SURROGATE: {
            cmpError(comp, tok, CMP_ERR_SESC_INVLD_LO_SRGT);
            break;
        }
        case SESC_8_INVALID_CP: {
            cmpError(comp, tok, CMP_ERR_SESC_INVALID_8_CP);
            break;
        }
        case SESC_NULL_PTR: {
            cmpError(comp, tok, CMP_ERR_SESC_NUL_PTR);
            break;
        }
    }
    return output;
}

// Compile Literal Expression
static bool compileLitExpr(PCompiler *comp, PExpr *expr) {
    struct ELiteral *lit = &expr->exp.ELiteral;

    switch (lit->type) {
        case EXP_LIT_NUM: {
            u16 constIdx = addConstant(comp, MakeNumber(lit->value.nvalue));
            emitBtU16(comp, lit->op, OP_CONST, constIdx);
            break;
        }
        case EXP_LIT_STR: {
            // TODO: ESCAPE String HERE
            Token *opTok = expr->op;
            char *escapedStr = readStringEscapes(comp, opTok);
            // We hand ownership of escaped str to the string object
            PObj *strObj = NewStrObject(comp->gc, expr->op, escapedStr, true);

            if (strObj == NULL) {
                cmpError(comp, expr->op, CMP_ERR_IME_FAIL_STROBJ_LIT);
                return false;
            }
            u16 constIdx = addConstant(comp, MakeObject(strObj));
            emitBtU16(comp, lit->op, OP_CONST, constIdx);
            break;
        }
        case EXP_LIT_BOOL: {
            emitBt(comp, lit->op, lit->value.bvalue ? OP_TRUE : OP_FALSE);
            break;
        }
        case EXP_LIT_NIL: {
            emitBt(comp, lit->op, OP_NIL);
            break;
        }

        default: {
            cmpError(comp, lit->op, CMP_ERR_INVALID_EXPR);
            return false;
            break;
        }
    }

    return true;
}

// Compile a Binary Expression
static bool compileBinExpr(PCompiler *comp, PExpr *expr) {
    struct EBinary *bin = &expr->exp.EBinary;

    if (!compileExpr(comp, bin->left)) {
        cmpError(comp, bin->left->op, CMP_ERR_LEFT_BIN_EXPR);
        return false;
    }

    if (!compileExpr(comp, bin->right)) {
        cmpError(comp, bin->right->op, CMP_ERR_RIGHT_BIN_EXPR);
        return false;
    }

    switch (bin->op->type) {
        case T_PLUS: emitBt(comp, bin->op, OP_ADD); break;
        case T_MINUS: emitBt(comp, bin->op, OP_SUB); break;
        case T_ASTR: emitBt(comp, bin->op, OP_MUL); break;
        case T_SLASH: emitBt(comp, bin->op, OP_DIV); break;
        case T_MOD: emitBt(comp, bin->op, OP_MOD); break;
        case T_EXPONENT: emitBt(comp, bin->op, OP_EXPONENT); break;
        case T_EQEQ: emitBt(comp, bin->op, OP_EQUAL); break;
        case T_BANG_EQ: emitBt(comp, bin->op, OP_NOTEQUAL); break;
        case T_GT: emitBt(comp, bin->op, OP_GT); break;
        case T_GTE: emitBt(comp, bin->op, OP_GTE); break;
        case T_LT: emitBt(comp, bin->op, OP_LT); break;
        case T_LTE: emitBt(comp, bin->op, OP_LTE); break;
        default: break;
    }

    return true;
}

// Compile a logical type expression such as AND , OR
//
// AND Bytecode structure
// <Left Expression>        [Left] in stack
// [OP_POP_JUMP_IF_FALSE] -> END  [Left] if is false, so [FALSE] otherwise pop
// -> [_] <Right Expression>        [Right] (if Left is false, we never reach
// here) End/Other Expression...  Result -> [TRUE/FALSE]
//
// OR Bytecode structure
// <Left Expression>        [Left] is in stack
// [OP_POP_JUMP_IF_TRUE] -> END  [Left] if is true, so [TRUE] otherwise pop ->
// [_] <Right Expression>       [Right] (if left is true, we never reach here)
// End/Other Expression
static bool compileLogicalExpr(PCompiler *comp, PExpr *expr) {
    struct ELogical *logic = &expr->exp.ELogical;

    if (!compileExpr(comp, logic->left)) {
        cmpError(comp, logic->left->op, CMP_ERR_LEFT_LGC_EXPR);
        return false;
    }

    if (logic->op->type == T_AND) {
        u16 endJump = emitJump(comp, logic->op, OP_POP_JUMP_IF_FALSE);
        if (!compileExpr(comp, logic->right)) {
            cmpError(comp, logic->right->op, CMP_ERR_RIGHT_LGC_EXPR);
            return false;
        }
        patchJump(comp, endJump);
    } else if (logic->op->type == T_OR) {
        u16 endJump = emitJump(comp, logic->op, OP_POP_JUMP_IF_TRUE);
        if (!compileExpr(comp, logic->right)) {
            cmpError(comp, logic->right->op, CMP_ERR_RIGHT_LGC_EXPR);
            return false;
        }
        patchJump(comp, endJump);
    }

    return true;
}

static bool compileUnaryExpr(PCompiler *comp, PExpr *expr) {
    struct EUnary *unary = &expr->exp.EUnary;

    if (!compileExpr(comp, unary->right)) {
        cmpError(comp, unary->right->op, CMP_ERR_UNARY_EXPR);
        return false;
    }
    switch (unary->op->type) {
        case T_MINUS: emitBt(comp, unary->op, OP_NEGATE); break;
        case T_BANG: emitBt(comp, unary->op, OP_NOT); break;
        default: break;
    }
    return true;
}

static bool compileArrayExpr(PCompiler *comp, PExpr *expr) {
    struct EArray *arr = &expr->exp.EArray;
    u64 itemCount = arr->count;
    for (u64 i = 0; i < itemCount; i++) {
        if (!compileExpr(comp, arr->items[i])) {
            cmpError(comp, arr->items[i]->op, CMP_ERR_ARR_ITEM_EXPR);
            return false;
        }
    }

    emitBtU16(comp, arr->op, OP_ARRAY, (u16)itemCount);

    return true;
}

static bool compileMapExpr(PCompiler *comp, PExpr *expr) {
    struct EMap *map = &expr->exp.EMap;
    u64 itemCount = map->count; // actual pairs = itemCount/2
    u64 pairCount = itemCount / 2;
    for (u64 i = 0; i < itemCount; i += 2) {
        if (!compileExpr(comp, map->etable[i])) {
            cmpError(comp, map->etable[i]->op, CMP_ERR_MAP_KEY_EXPR);
            return false;
        }

        if (!compileExpr(comp, map->etable[i + 1])) {
            cmpError(comp, map->etable[i + 1]->op, CMP_ERR_MAP_VAL_EXPR);
            return false;
        }
    }

    emitBtU16(comp, map->op, OP_MAP, (u16)pairCount);

    return true;
}

// Add a String Identifier to Constant Pool
// Make a string object, Make a value out of it, push the value to constant pool
// return the constant index
static u16 addIdentConst(PCompiler *comp, Token *tok) {

    PObj *strObj = NewStrObject(comp->gc, tok, tok->lexeme, false);
    if (strObj == NULL) {
        cmpError(comp, tok, CMP_ERR_IME_FAIL_STR_IDENT);
        return 0;
    }
    PValue strVal = MakeObject(strObj);
    u16 constIndex = addConstant(comp, strVal);
    return constIndex;
}

// Compile a variable expression
static bool compileVariableExpr(PCompiler *comp, PExpr *expr) {
    struct EVariable *var = &expr->exp.EVariable;

    int localIndex = findLocal(comp, var->name);
    if (localIndex != -1 && localIndex != -2) {
        emitBtU16(comp, var->name, OP_GET_LOCAL, localIndex);
        return true;
    }

    // look for local in enclosing scope or previosly made upvalues
    int upvalIndex = findUpvalue(comp, var->name);
    if (upvalIndex != -1) {
        emitBtU16(comp, var->name, OP_GET_UPVAL, upvalIndex);
        return true;
    }

    u16 constIndex = addIdentConst(comp, var->name);
    emitBtU16(comp, var->name, OP_GET_GLOBAL, constIndex);
    return true;
}

static bool cmpVariableAssign(PCompiler *comp, PExpr *expr) {
    struct EAssign *assign = &expr->exp.EAssign;
    if (!compileExpr(comp, assign->value)) {
        cmpError(comp, assign->op, CMP_ERR_ASN_VAL);
        return false;
    }

    int localIndex = findLocal(comp, assign->name->exp.EVariable.name);
    if (localIndex != -1 && localIndex != -2) {
        emitBtU16(comp, assign->op, OP_SET_LOCAL, localIndex);
        return true;
    }

    // same thing as finding enclosing local or previosly made upvalues
    Token *varName = assign->name->exp.EVariable.name;
    int upvalIndex = findUpvalue(comp, varName);
    if (upvalIndex != -1) {
        emitBtU16(comp, varName, OP_SET_UPVAL, upvalIndex);
    }

    u16 constIndex = addIdentConst(comp, assign->name->exp.EVariable.name);
    emitBtU16(comp, assign->op, OP_SET_GLOBAL, constIndex);
    return true;
}

static bool cmpSubsAssign(PCompiler *comp, PExpr *expr) {
    struct EAssign *assign = &expr->exp.EAssign;
    struct ESubscript *subExpr = &assign->name->exp.ESubscript;
    if (!compileExpr(comp, subExpr->value)) {
        cmpError(comp, subExpr->value->op, CMP_ERR_SUBS_ASN_VAL);
        return false;
    }

    if (!compileExpr(comp, subExpr->index)) {
        cmpError(comp, subExpr->index->op, CMP_ERR_SUBS_ASN_IDX);
        return false;
    }

    if (!compileExpr(comp, assign->value)) {
        cmpError(comp, assign->value->op, CMP_ERR_SUBS_ASN_ASNVAL);
        return false;
    }

    emitBt(comp, assign->op, OP_SUBS_ASSIGN);

    return true;
}

static bool compileAssignExpr(PCompiler *comp, PExpr *expr) {
    struct EAssign *assign = &expr->exp.EAssign;
    if (assign->name->type == EXPR_VARIABLE) {
        return cmpVariableAssign(comp, expr);
    } else if (assign->name->type == EXPR_SUBSCRIPT) {
        return cmpSubsAssign(comp, expr);
    }

    return false;
}

static bool compileCallExpr(PCompiler *comp, PExpr *expr) {
    struct ECall *call = &expr->exp.ECall;
    if (!compileExpr(comp, call->callee)) {
        cmpError(comp, call->callee->op, CMP_ERR_CALL_CALLEE);
        return false;
    }
    for (u64 i = 0; i < call->argCount; i++) {
        if (!compileExpr(comp, call->args[i])) {
            cmpError(comp, call->args[i]->op, CMP_ERR_CALL_ARG);
            return false;
        }
    }

    emitBtU16(comp, call->op, OP_CALL, (u16)call->argCount); // todo: call arg

    return true;
}

static bool compileSubscriptExpr(PCompiler *comp, PExpr *expr) {
    struct ESubscript *subExpr = &expr->exp.ESubscript;
    if (!compileExpr(comp, subExpr->value)) {
        cmpError(comp, subExpr->value->op, CMP_ERR_SUBS_VAL);
        return false;
    }

    if (!compileExpr(comp, subExpr->index)) {
        cmpError(comp, subExpr->index->op, CMP_ERR_SUBS_IDX);
        return false;
    }

    emitBt(comp, subExpr->op, OP_SUBSCRIPT);

    return true;
}

static bool compileModgetExpr(PCompiler *comp, PExpr *expr) {
    struct EModget *modgetExpr = &expr->exp.EModget;
    if (!compileExpr(comp, modgetExpr->module)) {
        cmpError(comp, modgetExpr->module->op, CMP_ERR_MOD_EXPR);
        return false;
    }

    u16 constIndex = addIdentConst(comp, modgetExpr->child);
    emitBtU16(comp, modgetExpr->op, OP_MODGET, constIndex);

    return true;
}

static bool compileExpr(PCompiler *comp, PExpr *expr) {
    if (expr == NULL) {
        return false;
    }
    switch (expr->type) {
        case EXPR_LITERAL: return compileLitExpr(comp, expr);
        case EXPR_BINARY: return compileBinExpr(comp, expr);
        case EXPR_LOGICAL: return compileLogicalExpr(comp, expr);
        case EXPR_UNARY: return compileUnaryExpr(comp, expr);
        case EXPR_ARRAY: return compileArrayExpr(comp, expr);
        case EXPR_MAP: return compileMapExpr(comp, expr);
        case EXPR_VARIABLE: return compileVariableExpr(comp, expr);
        case EXPR_ASSIGN: return compileAssignExpr(comp, expr);
        case EXPR_CALL: return compileCallExpr(comp, expr);
        case EXPR_SUBSCRIPT: return compileSubscriptExpr(comp, expr);
        case EXPR_MODGET: return compileModgetExpr(comp, expr);
        case EXPR_GROUPING: {
            struct EGrouping *grouping = &expr->exp.EGrouping;
            return compileExpr(comp, grouping->expr);
        }
    }

    return true;
}

static bool compileExprStmt(PCompiler *comp, PStmt *stmt) {
    struct SExpr *expr = &stmt->stmt.SExpr;
    if (!compileExpr(comp, stmt->stmt.SExpr.expr)) {
        cmpError(comp, expr->op, CMP_ERR_EXPR_STMT);
        return false;
    }

    emitBt(comp, stmt->stmt.SExpr.expr->op, OP_POP);
    return true;
}

static bool compileDebugStmt(PCompiler *comp, PStmt *stmt) {
    if (!compileExpr(comp, stmt->stmt.SDebug.expr)) {
        cmpError(comp, stmt->stmt.SDebug.expr->op, CMP_ERR_DBG_STMT);
        return false;
    }

    emitBt(comp, stmt->stmt.SDebug.expr->op, OP_DEBUG);
    return true;
}

// Check if name exists in current local scope
// returns index if found otherwise returns -1
static int doesLocalExists(PCompiler *comp, Token *name) {
    for (int i = comp->localCount - 1; i >= 0; i--) {
        PLocal *lcl = &comp->locals[i];
        if (lcl->depth != -1 && lcl->depth < comp->scopeDepth) {
            break;
        }

        if (lcl->name == NULL) {
            continue;
        }
        if (isIdentTokenEqual(name, lcl->name)) {
            return i;
        }
    }
    return -1;
}

// Try to Declare a local variable
// if we are in global scope, do nothing
// if we have a local variable with same name in the same scope return error
// otherwise create and add a new local variable
static void tryLocalDeclare(PCompiler *comp, Token *name) {
    if (comp->scopeDepth == 0) {
        return;
    }
    if (doesLocalExists(comp, name) != -1) {
        const char *errMsg = StrFormat(CMP_ERR_VAR_EXIST_SCOPE, name->lexeme);
        cmpError(comp, name, errMsg);
        return;
    }
    if (comp->localCount >= MAX_COMPILER_LOCAL_COUNT) {
        cmpError(comp, name, CMP_ERR_TOO_MANY_LOCAL);
        return; // todo error
    }
    PLocal *local = &comp->locals[comp->localCount++];
    local->name = name;
    local->depth = -1;
    // flag it as not upvalue for inner scopes by default
    local->isCaptured = false;
}

// if in global scope, we emit define global opcode to ask the vm to put the
// value in globals table.
// Otherwise, we must have added a local already from `tryLocalDeclare`
// so just mark it as usable
static void defineVariable(PCompiler *comp, u16 constIndex, Token *name) {
    if (comp->scopeDepth > 0) {
        markLocalInit(comp);
        return;
    }
    emitBtU16(comp, name, OP_DEFINE_GLOBAL, constIndex);
}

// Try setting up variable name.
// If is local, we create a local
// otherwise we create a Identifier constant from token
static u16 readVariableName(PCompiler *comp, Token *name) {
    tryLocalDeclare(comp, name);
    if (comp->scopeDepth > 0) {
        return 0;
    }
    return addIdentConst(comp, name);
}

static bool compileLetStmt(PCompiler *comp, PStmt *stmt) {
    struct SLet *let = &stmt->stmt.SLet;
    u16 globNameIndex = readVariableName(comp, let->name);
    if (!compileExpr(comp, let->expr)) {
        cmpError(comp, let->expr->op, CMP_ERR_LET_STMT);
        return false;
    }
    defineVariable(comp, globNameIndex, let->name);

    return true;
}

static bool compileBlockStmt(PCompiler *comp, PStmt *stmt) {
    startScope(comp);
    struct SBlock *block = &stmt->stmt.SBlock;
    u64 stmtCount = arrlen(block->stmts);
    for (u64 i = 0; i < stmtCount; i++) {
        if (!compileStmt(comp, block->stmts[i])) {
            endScope(comp, block->op);
            cmpError(comp, block->stmts[i]->op, CMP_ERR_BLK_STMT);
            return false;
        }
    }
    endScope(comp, block->op);
    return true;
}

static bool compileIfStmt(PCompiler *comp, PStmt *stmt) {
    struct SIf *ifstmt = &stmt->stmt.SIf;
    if (!compileExpr(comp, ifstmt->cond)) {
        cmpError(comp, ifstmt->cond->op, CMP_ERR_IF_COND);
        return false;
    }
    int thenJump = emitJump(comp, ifstmt->op, OP_JUMP_IF_FALSE);
    emitBt(comp, ifstmt->op, OP_POP);

    if (!compileStmt(comp, ifstmt->thenBranch)) {
        cmpError(comp, ifstmt->thenBranch->op, CMP_ERR_IF_THEN);
        return false;
    }

    int elseJump = emitJump(comp, ifstmt->op, OP_JUMP);
    patchJump(comp, thenJump);
    emitBt(comp, ifstmt->op, OP_POP);

    if (ifstmt->elseBranch != NULL) {
        if (!compileStmt(comp, ifstmt->elseBranch)) {
            cmpError(comp, ifstmt->elseBranch->op, CMP_ERR_IF_ELSE);
            return false;
        }
    }
    patchJump(comp, elseJump);

    return true;
}

static void emitLoop(PCompiler *comp, Token *token, u16 loopStart) {
    emitBt(comp, token, OP_LOOP);
    u16 offset = getbt(comp)->codeCount - loopStart + 2;

    EmitRawU16(getbt(comp), offset);
}

static PCompLoopCtx *enterLoop(PCompiler *comp, u16 loopStart) {
    PCompLoopCtx *loopCtx = PCreate(PCompLoopCtx);
    if (loopCtx == NULL) {
        return NULL;
    }
    loopCtx->breakJumps = NULL;
    loopCtx->loopStart = loopStart;
    loopCtx->enclosing = comp->loopCtx;
    comp->loopCtx = loopCtx;
    return loopCtx;
}

static void exitLoop(PCompiler *comp) {
    if (comp->loopCtx == NULL) {
        return;
    }

    PCompLoopCtx *loopCtx = comp->loopCtx;

    i64 breakCount = arrlen(loopCtx->breakJumps);
    for (i64 i = 0; i < breakCount; i++) {
        patchJump(comp, loopCtx->breakJumps[i]);
    }

    arrfree(loopCtx->breakJumps);
    loopCtx->breakJumps = NULL;
    loopCtx->loopStart = 0;
    comp->loopCtx = loopCtx->enclosing;

    PFree(loopCtx);
}

static bool compileWhileStmt(PCompiler *comp, PStmt *stmt) {
    struct SWhile *whileStmt = &stmt->stmt.SWhile;

    u16 loopStart = getbt(comp)->codeCount;

    PCompLoopCtx *loopCtx = enterLoop(comp, loopStart);
    if (loopCtx == NULL) {
        cmpError(comp, whileStmt->op, CMP_ERR_IME_FAIL_LPCTX_WHL);
        return false;
    }

    if (!compileExpr(comp, whileStmt->cond)) {
        cmpError(comp, whileStmt->cond->op, CMP_ERR_WHL_COND);
        return false;
    }

    u16 exitJump = emitJump(comp, whileStmt->op, OP_JUMP_IF_FALSE);

    emitBt(comp, whileStmt->op, OP_POP);

    if (!compileStmt(comp, whileStmt->body)) {
        cmpError(comp, whileStmt->body->op, CMP_ERR_WHL_BODY);
        return false;
    }

    emitLoop(comp, whileStmt->op, loopStart);

    patchJump(comp, exitJump);
    emitBt(comp, whileStmt->op, OP_POP);
    exitLoop(comp);
    return true;
}

static bool compileBreakStmt(PCompiler *comp, PStmt *stmt) {
    struct SBreak *breakStmt = &stmt->stmt.SBreak;

    if (comp->loopCtx == NULL) {
        cmpError(comp, breakStmt->op, CMP_ERR_IME_FAIL_ACS_LPCTX_BRK);
        return false;
    }
    u16 jumpPos = emitJump(comp, breakStmt->op, OP_JUMP);
    arrput(comp->loopCtx->breakJumps, jumpPos);
    return true;
}

static bool compileContinueStmt(PCompiler *comp, PStmt *stmt) {
    struct SContinue *contStmt = &stmt->stmt.SContinue;

    if (comp->loopCtx == NULL) {
        cmpError(comp, contStmt->op, CMP_ERR_IME_FAIL_ACS_LPCTX_CNT);
        return false;
    }
    emitLoop(comp, contStmt->op, comp->loopCtx->loopStart);
    return true;
}

static bool compileFuncBody(PCompiler *comp, PStmt **stmts) {
    u64 stmtCount = arrlen(stmts);
    for (u64 i = 0; i < stmtCount; i++) {
        if (!compileStmt(comp, stmts[i])) {
            cmpError(comp, stmts[i]->op, CMP_ERR_FNC_BODY_STMT);
            return false;
        }
    }

    emitReturn(comp);
    return true;
}

static bool compileFunc(PCompiler *comp, PStmt *stmt) {
    struct SFunc *fnStmt = &stmt->stmt.SFunc;
    PCompiler *fComp =
        NewEnclosedCompiler(comp->gc, comp, COMP_FN_FUNCTION, fnStmt->name);
    if (fComp == NULL) {
        cmpError(comp, fnStmt->name, CMP_ERR_IME_FAIL_FNC_CMP);
        return false;
    }

    startScope(fComp);
    for (u64 i = 0; i < fnStmt->paramCount; i++) {
        u16 paramIndex = readVariableName(fComp, fnStmt->params[i]);
        defineVariable(fComp, paramIndex, fnStmt->params[i]);
    }
    if (!compileFuncBody(fComp, fnStmt->body->stmt.SBlock.stmts)) {
        cmpError(comp, fnStmt->body->stmt.SBlock.op, CMP_ERR_FNC_BODY);
        return false;
    }
    PObj *fnObj = GetCompiledFunction(fComp);
    if (fnObj == NULL) {
        cmpError(comp, fnStmt->name, CMP_ERR_FNC_ACS_CMPFNC);
        return false;
    }
    fnObj->v.OComFunction.paramCount = fnStmt->paramCount;
    u16 constIndex = addConstant(comp, MakeObject(fnObj));
    emitBtU16(comp, fnStmt->name, OP_CLOSURE, constIndex);

    // emit informatin about function scope upvalues
    for (i16 i = 0; i < fnObj->v.OComFunction.upvalCount; i++) {
        EmitRawU16(getbt(comp), fComp->upvals[i].isLocal ? 1 : 0);
        EmitRawU16(getbt(comp), fComp->upvals[i].index);
    }

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_BYTECODE) {
        PanPrint("--------- %s --------\n", stmt->op->lexeme);
        DebugBytecode(fnObj->v.OComFunction.code, 0);
    }
#endif
    FreeCompiler(fComp);
    return true;
}

static bool compileFuncStmt(PCompiler *comp, PStmt *stmt) {
    struct SFunc *fnStmt = &stmt->stmt.SFunc;
    u16 identIndex = readVariableName(comp, fnStmt->name);
    markLocalInit(comp);
    compileFunc(comp, stmt);
    defineVariable(comp, identIndex, fnStmt->name);
    return true;
}

static bool compileReturnStmt(PCompiler *comp, PStmt *stmt) {
    struct SReturn *retStmt = &stmt->stmt.SReturn;
    if (comp->funcType == COMP_FN_SCRIPT) {
        cmpError(comp, retStmt->op, CMP_ERR_RET_TOP_LVL);
        return false;
    }
    if (retStmt->value != NULL) {
        if (!compileExpr(comp, retStmt->value)) {
            cmpError(comp, retStmt->value->op, CMP_ERR_RET_VAL);
            return false;
        }
    } else {
        emitBt(comp, retStmt->op, OP_NIL);
    }

    emitBt(comp, retStmt->op, OP_RETURN);
    return true;
}

static bool compileImportStmt(PCompiler *comp, PStmt *stmt) {
    struct SImport *importStmt = &stmt->stmt.SImport;
    u16 customNameIndex = readVariableName(comp, importStmt->name);
    if (!compileExpr(comp, importStmt->path)) {
        cmpError(comp, importStmt->op, CMP_ERR_IMPRT_PATH);
        return false;
    }

    emitBtU16(comp, importStmt->op, OP_IMPORT, customNameIndex);
    return true;
}

static bool compileStmt(PCompiler *comp, PStmt *stmt) {
    if (comp == NULL || stmt == NULL) {
        return false;
    }

    switch (stmt->type) {
        case STMT_EXPR: {
            return compileExprStmt(comp, stmt);
        }
        case STMT_DEBUG: {
            return compileDebugStmt(comp, stmt);
        }
        case STMT_LET: {
            return compileLetStmt(comp, stmt);
        }
        case STMT_BLOCK: {
            return compileBlockStmt(comp, stmt);
        }
        case STMT_IF: {
            return compileIfStmt(comp, stmt);
        }
        case STMT_WHILE: {
            return compileWhileStmt(comp, stmt);
        }
        case STMT_FUNC: {
            return compileFuncStmt(comp, stmt);
        }
        case STMT_RETURN: {
            return compileReturnStmt(comp, stmt);
        }
        case STMT_IMPORT: {
            return compileImportStmt(comp, stmt);
        }
        case STMT_BREAK: {
            return compileBreakStmt(comp, stmt);
        }
        case STMT_CONTINUE: {
            return compileContinueStmt(comp, stmt);
        }
    }

    return false;
}
