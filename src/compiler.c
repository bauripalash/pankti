#include "include/compiler.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/token.h"
#include "include/utils.h"
#include "include/vm.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_CONST_COUNT 65535

// Compile and Emit Bytecodes for a Parser Produced Statement
static bool compileStmt(PCompiler *comp, PStmt *stmt);
// Compiled and Emit Bytecodes for a Parser Produced Expression
static bool compileExpr(PCompiler *comp, PExpr *expr);
// Add a Constant and return its index in constant list
static u16 addConstant(PCompiler *comp, PValue value);

PCompiler *dummyCompiler(
    PanktiCore *core, PCompiler *enclosing, PCompFuncType ftype, Token *name
) {
    PCompiler *c = PCreate(PCompiler);
    c->func = NULL;
    c->funcType = ftype;
    c->enclosing = enclosing;
    c->prog = NULL;
    c->progCount = 0;
    c->scopeDepth = 0;
    c->localCount = 0;
    c->dummyToken = NewToken(T_EOF);
    if (core != NULL) {
        c->core = core;
        c->gc = core->gc;

        if (name != NULL && ftype == COMP_FN_FUNCTION) {
            c->func = NewComFuncObject(c->gc, name);
        } else {
            c->func = NewComFuncObject(c->gc, NULL);
        }

    } else {
        c->core = NULL;
        c->gc = NULL;
        c->func = NULL;
    }

    PLocal *local = &c->locals[c->localCount++];
    local->depth = 0;
    local->name = NULL;
    return c;
}

PCompiler *NewCompiler(PanktiCore *core) {
    PCompiler *c = dummyCompiler(core, NULL, COMP_FN_SCRIPT, NULL);

    return c;
}

PCompiler *NewEnclosedCompiler(
    PanktiCore *core, PCompiler *comp, PCompFuncType ftype, Token *name
) {

    PCompiler *c = dummyCompiler(core, comp, COMP_FN_FUNCTION, name);

    return c;
}

void FreeCompiler(PCompiler *comp) {
    if (comp == NULL) {
        return;
    }
    FreeToken(comp->dummyToken);
    PFree(comp);
}

static void cmpError(PCompiler *comp, Token *token, const char *msg) {
    CoreCompilerError(comp->core, token, msg);
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
        cmpError(comp, NULL, "Conditional Jump is too long");
        return; // todo: error
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
static int endScope(PCompiler *comp) {
    comp->scopeDepth--;

    while (comp->localCount > 0 &&
           comp->locals[comp->localCount - 1].depth > comp->scopeDepth) {
        emitBt(comp, getbt(comp)->tokens[0], OP_POP);
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
                cmpError(
                    comp, name,
                    "Cannot read variable in its own init Statement\n"
                );
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
            return false;
        }
    }
    endCompiler(compiler);

    return true;
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
            PObj *strObj =
                NewStrObject(comp->gc, expr->op, lit->value.svalue, false);
            if (strObj == NULL) {
                // todo: error check
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
            // todo: invalid literal error
            break;
        }
    }

    return true;
}

// Compile a Binary Expression
static bool compileBinExpr(PCompiler *comp, PExpr *expr) {
    struct EBinary *bin = &expr->exp.EBinary;

    if (!compileExpr(comp, bin->left)) {
        return false;
    }

    if (!compileExpr(comp, bin->right)) {
        return false;
    }

    switch (bin->op->type) {
        case T_PLUS: emitBt(comp, bin->op, OP_ADD); break;
        case T_MINUS: emitBt(comp, bin->op, OP_SUB); break;
        case T_ASTR: emitBt(comp, bin->op, OP_MUL); break;
        case T_SLASH: emitBt(comp, bin->op, OP_DIV); break;
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

    compileExpr(comp, logic->left);

    if (logic->op->type == T_AND) {
        u16 endJump = emitJump(comp, logic->op, OP_POP_JUMP_IF_FALSE);
        compileExpr(comp, logic->right);
        patchJump(comp, endJump);
    } else if (logic->op->type == T_OR) {
        u16 endJump = emitJump(comp, logic->op, OP_POP_JUMP_IF_TRUE);
        compileExpr(comp, logic->right);
        patchJump(comp, endJump);
    }

    return true;
}

static bool compileUnaryExpr(PCompiler *comp, PExpr *expr) {
    struct EUnary *unary = &expr->exp.EUnary;

    if (!compileExpr(comp, unary->right)) {
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
            return false;
        }

        if (!compileExpr(comp, map->etable[i + 1])) {
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
    PValue strVal = MakeObject(strObj);
    u16 constIndex = addConstant(comp, strVal);
    return constIndex;
}

// Compile a variable expression
static bool compileVariableExpr(PCompiler *comp, PExpr *expr) {
    struct EVariable *var = &expr->exp.EVariable;

    int localIndex = findLocal(comp, var->name);
    if (localIndex != -1) {
        emitBtU16(comp, var->name, OP_GET_LOCAL, localIndex);
        return true;
    }

    u16 constIndex = addIdentConst(comp, var->name);
    emitBtU16(comp, var->name, OP_GET_GLOBAL, constIndex);
    return true;
}

static bool compileAssignExpr(PCompiler *comp, PExpr *expr) {
    struct EAssign *assign = &expr->exp.EAssign;
    if (assign->name->type == EXPR_VARIABLE) {
        compileExpr(comp, assign->value);

        int localIndex = findLocal(comp, assign->name->exp.EVariable.name);
        if (localIndex != -1) {
            emitBtU16(comp, assign->op, OP_SET_LOCAL, localIndex);
            return true;
        }

        u16 constIndex = addIdentConst(comp, assign->name->exp.EVariable.name);
        emitBtU16(comp, assign->op, OP_SET_GLOBAL, constIndex);
        return true;
    }

    return false;
}

static bool compileCallExpr(PCompiler *comp, PExpr *expr) {
    struct ECall *call = &expr->exp.ECall;
    if (!compileExpr(comp, call->callee)) {
        return true;
    }
    for (u64 i = 0; i < call->argCount; i++) {
        if (!compileExpr(comp, call->args[i])) {
            return false;
        }
    }

    emitBtU16(comp, call->op, OP_CALL, (u16)call->argCount); // todo: call arg

    return true;
}

static bool compileExpr(PCompiler *comp, PExpr *expr) {
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
        default: break;
    }

    return true;
}

static bool compileExprStmt(PCompiler *comp, PStmt *stmt) {
    if (!compileExpr(comp, stmt->stmt.SExpr.expr)) {
        return false;
    }

    emitBt(comp, stmt->stmt.SExpr.expr->op, OP_POP);
    return true;
}

static bool compileDebugStmt(PCompiler *comp, PStmt *stmt) {
    if (!compileExpr(comp, stmt->stmt.SDebug.expr)) {
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
        // PanPrint("Same variable exists in this scope -> %s\n", name->lexeme);
        const char *errMsg =
            StrFormat("Same variable exists in this scope : %s", name->lexeme);
        cmpError(comp, name, errMsg);
        return;
    }
    if (comp->localCount >= MAX_COMPILER_LOCAL_COUNT) {
        return; // todo error
    }
    PLocal *local = &comp->locals[comp->localCount++];
    local->name = name;
    local->depth = -1;
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
    compileExpr(comp, let->expr);
    defineVariable(comp, globNameIndex, let->name);

    return true;
}

static bool compileBlockStmt(PCompiler *comp, PStmt *stmt) {
    startScope(comp);
    struct SBlock *block = &stmt->stmt.SBlock;
    u64 stmtCount = arrlen(block->stmts);
    for (u64 i = 0; i < stmtCount; i++) {
        if (!compileStmt(comp, block->stmts[i])) {
            endScope(comp);
            return false;
        }
    }
    endScope(comp);
    return true;
}

static bool compileIfStmt(PCompiler *comp, PStmt *stmt) {
    struct SIf *ifstmt = &stmt->stmt.SIf;
    compileExpr(comp, ifstmt->cond);
    int thenJump = emitJump(comp, ifstmt->op, OP_JUMP_IF_FALSE);
    emitBt(comp, ifstmt->op, OP_POP);

    compileStmt(comp, ifstmt->thenBranch);

    int elseJump = emitJump(comp, ifstmt->op, OP_JUMP);
    patchJump(comp, thenJump);
    emitBt(comp, ifstmt->op, OP_POP);

    if (ifstmt->elseBranch != NULL) {
        compileStmt(comp, ifstmt->elseBranch);
    }
    patchJump(comp, elseJump);

    return true;
}

static void emitLoop(PCompiler *comp, Token *token, u16 loopStart) {
    emitBt(comp, token, OP_LOOP);
    u16 offset = getbt(comp)->codeCount - loopStart + 2;

    EmitRawU16(getbt(comp), offset);
}

static bool compileWhileStmt(PCompiler *comp, PStmt *stmt) {
    struct SWhile *whileStmt = &stmt->stmt.SWhile;
    u16 loopStart = getbt(comp)->codeCount;

    compileExpr(comp, whileStmt->cond);

    u16 exitJump = emitJump(comp, whileStmt->op, OP_JUMP_IF_FALSE);

    emitBt(comp, whileStmt->op, OP_POP);

    compileStmt(comp, whileStmt->body);

    emitLoop(comp, whileStmt->op, loopStart);

    patchJump(comp, exitJump);
    emitBt(comp, whileStmt->op, OP_POP);
    return true;
}

static bool compileFuncBody(PCompiler *comp, PStmt **stmts) {
    u64 stmtCount = arrlen(stmts);
    for (u64 i = 0; i < stmtCount; i++) {
        if (!compileStmt(comp, stmts[i])) {
            return false;
        }
    }

    emitReturn(comp);
    return true;
}

static bool compileFunc(PCompiler *comp, PStmt *stmt) {
    struct SFunc *fnStmt = &stmt->stmt.SFunc;
    PCompiler *fComp =
        NewEnclosedCompiler(comp->core, comp, COMP_FN_FUNCTION, fnStmt->name);

    startScope(fComp);
    for (u64 i = 0; i < fnStmt->paramCount; i++) {
        u16 paramIndex = readVariableName(fComp, fnStmt->params[i]);
        defineVariable(fComp, paramIndex, fnStmt->params[i]);
    }
    compileFuncBody(fComp, fnStmt->body->stmt.SBlock.stmts);
    PObj *fnObj = GetCompiledFunction(fComp);
    fnObj->v.OComFunction.paramCount = fnStmt->paramCount;
    u16 constIndex = addConstant(comp, MakeObject(fnObj));
    emitBtU16(comp, fnStmt->name, OP_CONST, constIndex);
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
        cmpError(comp, retStmt->op, "Top level script cannot contain return");
        return false;
    }
    if (retStmt->value != NULL) {
        compileExpr(comp, retStmt->value);
    } else {
        emitBt(comp, retStmt->op, OP_NIL);
    }

    emitBt(comp, retStmt->op, OP_RETURN);
    return false;
}

static bool compileStmt(PCompiler *comp, PStmt *stmt) {
    if (comp == NULL || stmt == NULL) {
        return false;
    }

    switch (stmt->type) {
        case STMT_EXPR: {
            compileExprStmt(comp, stmt);
            break;
        }
        case STMT_DEBUG: {
            compileDebugStmt(comp, stmt);
            break;
        }
        case STMT_LET: {
            compileLetStmt(comp, stmt);
            break;
        }
        case STMT_BLOCK: {
            compileBlockStmt(comp, stmt);
            break;
        }
        case STMT_IF: {
            compileIfStmt(comp, stmt);
            break;
        }
        case STMT_WHILE: {
            compileWhileStmt(comp, stmt);
            break;
        }
        case STMT_FUNC: {
            compileFuncStmt(comp, stmt);
            break;
        }
        case STMT_RETURN: {
            compileReturnStmt(comp, stmt);
            break;
        }
        default: {
            PanPrint("Unsupported Statement (Yet)\n");
            break;
        }
    }

    return true;
}
