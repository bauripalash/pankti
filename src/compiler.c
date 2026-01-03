#include "include/compiler.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/token.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_CONST_COUNT 65535

static bool compileStmt(PCompiler *comp, PStmt *stmt);
static bool compileExpr(PCompiler *comp, PExpr *expr);
static u16 addConstant(PCompiler *comp, PValue value);

PCompiler *NewCompiler(Pgc *gc) {
    PCompiler *c = PCreate(PCompiler);
    c->code = NewBytecode();
    c->prog = NULL;
    c->progCount = 0;
    c->gc = gc;
    c->scopeDepth = 0;
    c->localCount = 0;
    return c;
}
void FreeCompiler(PCompiler *comp) {
    if (comp->code != NULL) {
        FreeBytecode(comp->code);
    }

    PFree(comp);
}

static PBytecode *getCurBytecode(PCompiler *comp) { return comp->code; }

static u16 addConstant(PCompiler *comp, PValue value) {
    u16 index = AddConstantToPool(comp->code, value);
    return index;
}

static u64 emitBt(PCompiler *comp, Token *tok, PanOpCode op) {
    return EmitBytecode(comp->code, tok, op);
}

static u64 emitBtU16(PCompiler *comp, Token *tok, PanOpCode op, u16 a) {
    return EmitBytecodeWithOneArg(comp->code, tok, op, a);
}

static u16 emitJump(PCompiler *comp, Token *tok, PanOpCode op) {
    emitBtU16(comp, tok, op, 0xffff);
    return comp->code->codeCount - 2;
}

static void patchJump(PCompiler *comp, int offset) {
    int jump = comp->code->codeCount - offset - 2;
    if (jump > UINT16_MAX) {
        return; // todo: error
    }

    comp->code->code[offset] = (jump >> 8) & 0xff;
    comp->code->code[offset + 1] = jump & 0xff;
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
        emitBt(comp, comp->code->tokens[0], OP_POP);
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

        if (isIdentTokenEqual(name, lcl->name)) {
            if (lcl->depth == -1) {
                PanPrint("Cannot read variable in its own init Statement\n");
                return -2;
            }
            return i;
        }
    }
    return -1;
}

bool CompilerCompile(PCompiler *compiler, PStmt **prog) {
    compiler->prog = prog;
    compiler->progCount = (u64)arrlen(prog);

    for (u64 i = 0; i < compiler->progCount; i++) {
        if (!compileStmt(compiler, prog[i])) {
            return false;
        }
    }

    emitBt(compiler, compiler->code->tokens[0], OP_RETURN);

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
        PanPrint("Same variable exists in this scope -> %s\n", name->lexeme);
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
        comp->locals[comp->localCount - 1].depth = comp->scopeDepth;
        return;
    }
    emitBtU16(comp, name, OP_DEFINE_GLOBAL, constIndex);
}

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
    u16 offset = comp->code->codeCount - loopStart + 2;

    EmitRawU16(comp->code, offset);
}

static bool compileWhileStmt(PCompiler *comp, PStmt *stmt) {
    struct SWhile *whileStmt = &stmt->stmt.SWhile;
    u16 loopStart = comp->code->codeCount;

    compileExpr(comp, whileStmt->cond);

    u16 exitJump = emitJump(comp, whileStmt->op, OP_JUMP_IF_FALSE);

    emitBt(comp, whileStmt->op, OP_POP);

    compileStmt(comp, whileStmt->body);

    emitLoop(comp, whileStmt->op, loopStart);

    patchJump(comp, exitJump);
    emitBt(comp, whileStmt->op, OP_POP);
    return true;
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
        default: {
            PanPrint("Unsupported Statement (Yet)\n");
            break;
        }
    }

    return true;
}
