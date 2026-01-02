#include "include/opcode.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/token.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

static const POpDefinition opDefs[] = {
    {"OpConst", 1, {2}},        {"OpDebug", 0, {0}},
    {"OpReturn", 0, {0}},       {"OpTrue", 0, {0}},
    {"OpFalse", 0, {0}},        {"OpNil", 0, {0}},
    {"OpPop", 0, {0}},          {"OpAdd", 0, {0}},
    {"OpSub", 0, {0}},          {"OpMul", 0, {0}},
    {"OpDiv", 0, {0}},          {"OpExponent", 0, {0}},
    {"OpEqual", 0, {0}},        {"OpNotEqual", 0, {0}},
    {"OpGT", 0, {0}},           {"OpGTE", 0, {0}},
    {"OpLT", 0, {0}},           {"OpLTE", 0, {0}},
    {"OpNegate", 0, {0}},       {"OpNot", 0, {0}},
    {"OpArray", 1, {2}}, // todo: max u64 count
    {"OpMap", 1, {2}},   // todo max u64/2 count;
    {"OpDefineGlobal", 1, {2}}, {"OpGetGlobal", 1, {2}},
    {"OpSetGlobal", 1, {2}},    {"OpGetLocal", 1, {2}},
    {"OpSetLocal", 1, {2}},     {"OpJumpIfFalse", 1, {2}},
    {"OpJump", 1, {2}},
};

const char *OpCodeToStr(PanOpCode code) { return opDefs[code].name; }

POpDefinition GetOpDefinition(PanOpCode code) { return opDefs[code]; }

PBytecode *NewBytecode(void) {
    PBytecode *b = PCreate(PBytecode);
    b->code = NULL;
    b->codeCount = 0;
    b->constPool = NULL;
    b->constCount = 0;
    b->lines = NULL;
    b->tokens = NULL;
    return b;
}

void FreeBytecode(PBytecode *b) {
    if (b == NULL) {
        return;
    }

    if (b->code != NULL) {
        arrfree(b->code);
        b->codeCount = 0;
    }

    if (b->constPool != NULL) {
        arrfree(b->constPool);
        b->constCount = 0;
    }

    if (b->lines != NULL) {
        arrfree(b->lines);
        b->lines = NULL;
    }

    if (b->tokens != NULL) {
        arrfree(b->tokens);
        b->tokens = NULL;
    }

    PFree(b);
}

static u64 disasmSimpleIns(const char *name, u64 offset) {
    PanPrint("%s\n", name);
    return offset + 1;
}

static u64 disasmConstIns(const char *name, u64 offset, const PBytecode *b) {
    u16 constIndex = ReadU16(b, offset + 1);

    PanPrint("%s %d", name, constIndex);
    if (b->constPool != NULL) {
        PanPrint(" : ");
        PrintValue(b->constPool[constIndex]);
    }
    PanPrint("\n");
    return offset + 3;
}

static u64 disasmBytesIns(const char *name, u64 offset, const PBytecode *b) {
    PanPrint("%s", name);
    u16 itemCount = ReadU16(b, offset + 1);

    PanPrint(" [%d]", itemCount);
    PanPrint("\n");
    return offset + 2;
}

static u64 disasmJumpIns(
    const char *name, u64 offset, int sign, const PBytecode *b
) {
    u16 jump = ReadU16(b, offset + 1);
    PanPrint("%s : %05d -> %05d\n", name, offset, offset + 3 + sign * jump);

    return offset + 3;
}

static u64 disasmComplexDSIns(
    const char *name, u64 offset, const PBytecode *b
) {
    u16 count = (u16)b->code[offset + 1] << 8;
    count |= b->code[offset + 2];
    PanPrint("%s : %02d\n", name, count);
    return offset + 2;
}

u64 DisasmBytecode(const PBytecode *bt, u64 offset) {
    PanPrint("%05d ", offset);
    PanOpCode op = (PanOpCode)bt->code[offset];
    POpDefinition def = GetOpDefinition(op);

    switch (op) {
        case OP_RETURN:
        case OP_POP:
        case OP_TRUE:
        case OP_FALSE:
        case OP_NIL:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_EXPONENT:
        case OP_EQUAL:
        case OP_NOTEQUAL:
        case OP_GT:
        case OP_GTE:
        case OP_LT:
        case OP_LTE:
        case OP_NEGATE:
        case OP_DEBUG:
        case OP_NOT: {
            return disasmSimpleIns(def.name, offset);
        }

        case OP_CONST:
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL: {
            return disasmConstIns(def.name, offset, bt);
        }

        case OP_JUMP_IF_FALSE:
        case OP_JUMP: {
            return disasmJumpIns(def.name, offset, 1, bt);
        }

        case OP_SET_LOCAL:
        case OP_GET_LOCAL: {
            return disasmBytesIns(def.name, offset, bt);
        }
        case OP_MAP:
        case OP_ARRAY: {
            return disasmComplexDSIns(def.name, offset, bt);
        }
        default: {
            return offset + 1;
        }
    }
}

void DebugBytecode(const PBytecode *bt, u64 offset) {
    PanPrint("==== DEBUG BYTECODE ====\n");

    u64 i = 0;
    while (i < bt->codeCount) {
        i = DisasmBytecode(bt, i);
    }

    PanPrint("====  END BYTECODE  ====\n");
}

u64 EmitBytecode(PBytecode *b, Token *tok, PanOpCode op) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    arrput(b->tokens, tok);
    arrput(b->lines, tok->line);
    b->codeCount++;
    return pos;
}
u64 EmitBytecodeWithOneArg(PBytecode *b, Token *tok, PanOpCode op, u16 a) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, (u8)((a >> 8) & 0xFF));
    b->codeCount++;
    arrput(b->code, (u8)(a & 0xFF));
    b->codeCount++;
    arrput(b->tokens, tok);
    arrput(b->lines, tok->line);
    return pos;
}
u64 EmitBytecodeWithTwoArgs(
    PBytecode *b, Token *tok, PanOpCode op, u8 one, u8 two
) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, one);
    b->codeCount++;
    arrput(b->code, two);
    b->codeCount++;
    arrput(b->tokens, tok);
    arrput(b->lines, tok->line);
    return pos;
}

u16 AddConstantToPool(PBytecode *b, PValue value) {
    u16 index = b->constCount;
    if (index >= MAX_CONST_COUNT) {
        return UINT16_MAX;
    }

    arrput(b->constPool, value);
    b->constCount++;
    return index;
}

u16 ReadU16(const PBytecode *b, u64 offset) {
    if (b == NULL || offset >= b->codeCount) {
        return 0;
    }

    return (u16)((u16)(b->code[offset] << 8) | (u16)b->code[offset + 1]);
}

u16 ReadU16RawCode(const u8 *code, u64 offset) {
    return (u16)((u16)(code[offset] << 8) | (u16)code[offset + 1]);
}
