#include "include/opcode.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

static const POpDefinition opDefs[] = {
    {"OpConst", 1, {2}},    {"OpTrue", 0, {0}},   {"OpFalse", 0, {0}},
    {"OpNil", 0, {0}},      {"OpPop", 0, {0}},    {"OpAdd", 0, {0}},
    {"OpSub", 0, {0}},      {"OpMul", 0, {0}},    {"OpDiv", 0, {0}},
    {"OpExponent", 0, {0}}, {"OpEqual", 0, {0}},  {"OpNotEqual", 0, {0}},
    {"OpGT", 0, {0}},       {"OpGTE", 0, {0}},    {"OpLT", 0, {0}},
    {"OpLTE", 0, {0}},      {"OpNegate", 0, {0}}, {"OpNot", 0, {0}},
    {"OpArray", 1, {2}}, // todo: max u64 count
};

const char *OpCodeToStr(PanOpCode code) { return opDefs[code].name; }

POpDefinition GetOpDefinition(PanOpCode code) { return opDefs[code]; }

PBytecode *NewBytecode(void) {
    PBytecode *b = PCreate(PBytecode);
    b->code = NULL;
    b->codeCount = 0;
    b->constPool = NULL;
    b->constCount = 0;
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

    PFree(b);
}

static u64 disasmSimpleIns(const char *name, u64 offset) {
    PanPrint("%s\n", name);
    return 0;
}

static u64 disasmConstIns(const char *name, u64 offset, const PBytecode *b) {
    PanPrint("%s", name);
    u16 constIndex = ReadU16(b, offset + 1);
    if (b->constPool != NULL) {
        PanPrint(" : ");
        PrintValue(b->constPool[constIndex]);
    }
    PanPrint("\n");
    return 2;
}

static u64 disasmBytesIns(const char *name, u64 offset, const PBytecode *b) {
    PanPrint("%s", name);
    u16 itemCount = ReadU16(b, offset + 1);

    PanPrint(" [%d]", itemCount);
    PanPrint("\n");
    return 2;
}

void DebugBytecode(const PBytecode *bt, u64 offset) {
    PanPrint("==== DEBUG BYTECODE ====\n");
    u64 ofs = offset;
    while (ofs < bt->codeCount) {
        PanOpCode op = (PanOpCode)bt->code[ofs];
        POpDefinition def = GetOpDefinition(op);
        PanPrint("%05d : 0x%02x : ", ofs, op);
        switch (op) {
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
            case OP_NOT: {
                ofs += disasmSimpleIns(def.name, ofs);
                break;
            }
            case OP_CONST: {
                ofs += disasmConstIns(def.name, ofs, bt);
                break;
            }
            case OP_ARRAY: {
                ofs += disasmBytesIns(def.name, ofs, bt);
                break;
            }
        }
        ofs++;
    }

    PanPrint("====  END BYTECODE  ====\n");
}

u64 EmitBytecode(PBytecode *b, PanOpCode op) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    return pos;
}
u64 EmitBytecodeWithOneArg(PBytecode *b, PanOpCode op, u16 a) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, (u8)((a >> 8) & 0xFF));
    b->codeCount++;
    arrput(b->code, (u8)(a & 0xFF));
    b->codeCount++;
    return pos;
}
u64 EmitBytecodeWithTwoArgs(PBytecode *b, PanOpCode op, u8 one, u8 two) {
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, one);
    b->codeCount++;
    arrput(b->code, two);
    b->codeCount++;
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
