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
    return c;
}
void FreeCompiler(PCompiler *comp) {
    if (comp->code != NULL) {
        FreeBytecode(comp->code);
    }

    PFree(comp);
}

bool CompilerCompile(PCompiler *compiler, PStmt **prog) {
    compiler->prog = prog;
    compiler->progCount = (u64)arrlen(prog);

    for (u64 i = 0; i < compiler->progCount; i++) {
        if (!compileStmt(compiler, prog[i])) {
            return false;
        }
    }
    return false;
}

static u16 addConstant(PCompiler *comp, PValue value) {
    u16 index = AddConstantToPool(comp->code, value);
    return index;
}

static u64 emitBt(PCompiler *comp, PanOpCode op) {
    return EmitBytecode(comp->code, op);
}

static u64 emitBtU16(PCompiler *comp, PanOpCode op, u16 a) {
    return EmitBytecodeWithOneArg(comp->code, op, a);
}

// static u64 emitBtU8s(PCompiler * comp, PanOpCode op, u8 a, u8 b){
//     return EmitBytecodeWithTwoArgs(comp->code, op, a, b);
// }

static bool compileLitExpr(PCompiler *comp, PExpr *expr) {
    struct ELiteral *lit = &expr->exp.ELiteral;

    switch (lit->type) {
        case EXP_LIT_NUM: {
            u16 constIdx = addConstant(comp, MakeNumber(lit->value.nvalue));
            emitBtU16(comp, OP_CONST, constIdx);
            break;
        }
        case EXP_LIT_STR: {
            PObj *strObj =
                NewStrObject(comp->gc, expr->op, lit->value.svalue, false);
            if (strObj == NULL) {
                // todo: error check
            }
            u16 constIdx = addConstant(comp, MakeObject(strObj));
            emitBtU16(comp, OP_CONST, constIdx);
            break;
        }
        case EXP_LIT_BOOL: {
            emitBt(comp, lit->value.bvalue ? OP_TRUE : OP_FALSE);
            break;
        }
        case EXP_LIT_NIL: {
            emitBt(comp, OP_NIL);
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
        case T_PLUS: emitBt(comp, OP_ADD); break;
        case T_MINUS: emitBt(comp, OP_SUB); break;
        case T_ASTR: emitBt(comp, OP_MUL); break;
        case T_SLASH: emitBt(comp, OP_DIV); break;
        case T_EXPONENT: emitBt(comp, OP_EXPONENT); break;
        case T_EQEQ: emitBt(comp, OP_EQUAL); break;
        case T_BANG_EQ: emitBt(comp, OP_NOTEQUAL); break;
        case T_GT: emitBt(comp, OP_GT); break;
        case T_GTE: emitBt(comp, OP_GTE); break;
        case T_LT: emitBt(comp, OP_LT); break;
        case T_LTE: emitBt(comp, OP_LTE); break;
        default: break;
    }

    return true;
}

static bool compileUnaryExpr(PCompiler *comp, PExpr *expr) {
    struct EUnary *unary = &expr->exp.EUnary;

    if (!compileExpr(comp, unary->right)) {
        return false;
    }
    switch (unary->op->type) {
        case T_MINUS: emitBt(comp, OP_NEGATE); break;
        case T_BANG: emitBt(comp, OP_NOT); break;
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

    emitBtU16(comp, OP_ARRAY, (u16)itemCount);

    return true;
}

static bool compileExpr(PCompiler *comp, PExpr *expr) {
    switch (expr->type) {
        case EXPR_LITERAL: return compileLitExpr(comp, expr);
        case EXPR_BINARY: return compileBinExpr(comp, expr);
        case EXPR_UNARY: return compileUnaryExpr(comp, expr);
        case EXPR_ARRAY: return compileArrayExpr(comp, expr);
        default: break;
    }

    return true;
}

static bool compileExprStmt(PCompiler *comp, PStmt *stmt) {
    if (!compileExpr(comp, stmt->stmt.SExpr.expr)) {
        return false;
    }

    emitBt(comp, OP_POP);
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
        default: {
            PanPrint("Unsupported Statement (Yet)\n");
            break;
        }
    }

    return true;
}
