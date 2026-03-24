#include "include/opcode.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/terminal.h"
#include "include/token.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static const POpDefinition opDefs[] = {
    [OP_CONST] = {"OpConst", 1, {2}},
    [OP_DEBUG] = {"OpDebug", 0, {0}},
    [OP_RETURN] = {"OpReturn", 0, {0}},
    [OP_TRUE] = {"OpTrue", 0, {0}},
    [OP_FALSE] = {"OpFalse", 0, {0}},
    [OP_NIL] = {"OpNil", 0, {0}},
    [OP_POP] = {"OpPop", 0, {0}},
    [OP_ADD] = {"OpAdd", 0, {0}},
    [OP_SUB] = {"OpSub", 0, {0}},
    [OP_MUL] = {"OpMul", 0, {0}},
    [OP_DIV] = {"OpDiv", 0, {0}},
    [OP_MOD] = {"OpMod", 0, {0}},
    [OP_EXPONENT] = {"OpExponent", 0, {0}},
    [OP_EQUAL] = {"OpEqual", 0, {0}},
    [OP_NOTEQUAL] = {"OpNotEqual", 0, {0}},
    [OP_GT] = {"OpGT", 0, {0}},
    [OP_GTE] = {"OpGTE", 0, {0}},
    [OP_LT] = {"OpLT", 0, {0}},
    [OP_LTE] = {"OpLTE", 0, {0}},
    [OP_NEGATE] = {"OpNegate", 0, {0}},
    [OP_NOT] = {"OpNot", 0, {0}},
    [OP_ARRAY] = {"OpArray", 1, {2}}, // todo: max u64 count
    [OP_MAP] = {"OpMap", 1, {2}},     // todo max u64/2 count;
    [OP_DEFINE_GLOBAL] = {"OpDefineGlobal", 1, {2}},
    [OP_GET_GLOBAL] = {"OpGetGlobal", 1, {2}},
    [OP_SET_GLOBAL] = {"OpSetGlobal", 1, {2}},
    [OP_GET_LOCAL] = {"OpGetLocal", 1, {2}},
    [OP_SET_LOCAL] = {"OpSetLocal", 1, {2}},
    [OP_GET_UPVAL] = {"OpGetUpVal", 1, {2}},
    [OP_SET_UPVAL] = {"OpSetUpVal", 1, {2}},
    [OP_JUMP_IF_FALSE] = {"OpJumpIfFalse", 1, {2}},
    [OP_JUMP] = {"OpJump", 1, {2}},
    [OP_POP_JUMP_IF_FALSE] = {"OpPopJumpIfFalse", 1, {2}},
    [OP_POP_JUMP_IF_TRUE] = {"OpPopJumpIfTrue", 1, {2}},
    [OP_LOOP] = {"OpLoop", 1, {2}},
    [OP_CALL] = {"OpCall", 1, {2}},
    [OP_CLOSURE] = {"OpClosure", 1, {2}},
    [OP_SUBSCRIPT] = {"OpSubscript", 0, {0}},
    [OP_SUBS_ASSIGN] = {"OpSubsAssign", 0, {0}},
    [OP_IMPORT] = {"OpImport", 1, {2}},
    [OP_MODGET] = {"OpModGet", 1, {2}},
};

const char *OpCodeToStr(PanOpCode code) { return opDefs[code].name; }

POpDefinition GetOpDefinition(PanOpCode code) { return opDefs[code]; }

PBytecode *NewBytecode(void) {
    PBytecode *b = PCreate(PBytecode);
    b->code = NULL;
    b->codeCount = 0;
    b->constPool = NULL;
    b->constCount = 0;
    b->posTable = NULL;
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

    if (b->posTable != NULL) {
        arrfree(b->posTable);
        b->posTable = NULL;
    }

    PFree(b);
}

static u64 disasmSimpleIns(const char *name, u64 offset) {
    PanPrint("%s%s%s\n", TermGreen(), name, TermReset());
    return offset + 1;
}

static u64 disasmConstIns(const char *name, u64 offset, const PBytecode *b) {
    u16 constIndex = ReadU16(b, offset + 1);

    PanPrint("%s%s%s", TermGreen(), name, TermReset());
    PanPrint(" %d", constIndex);
    if (b->constPool != NULL) {
        PanPrint(" : ");
        PanPrint(TermPurple());
        PrintValue(b->constPool[constIndex]);
    }
    PanPrint(TermReset());
    PanPrint("\n");
    return offset + 3;
}

static u64 disasmBytesIns(const char *name, u64 offset, const PBytecode *b) {
    PanPrint("%s%s%s", TermGreen(), name, TermReset());
    u16 itemCount = ReadU16(b, offset + 1);

    PanPrint(" [%d]", itemCount);
    PanPrint("\n");
    return offset + 3;
}

static u64 disasmJumpIns(
    const char *name, u64 offset, int sign, const PBytecode *b
) {
    u16 jump = ReadU16(b, offset + 1);

    PanPrint("%s%s%s", TermGreen(), name, TermReset());
    PanPrint(" : %05d -> %05d\n", offset, offset + 3 + sign * jump);

    return offset + 3;
}

static u64 disasmComplexDSIns(
    const char *name, u64 offset, const PBytecode *b
) {
    u16 count = (u16)b->code[offset + 1] << 8;
    count |= b->code[offset + 2];

    PanPrint("%s%s%s", TermGreen(), name, TermReset());
    PanPrint(" : %02d\n", count);
    return offset + 3;
}

u64 DisasmBytecode(const PBytecode *bt, u64 offset) {
    PanPrint("%s%05d %s", TermBlue(), offset, TermReset());
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
        case OP_MOD:
        case OP_EXPONENT:
        case OP_EQUAL:
        case OP_NOTEQUAL:
        case OP_GT:
        case OP_GTE:
        case OP_LT:
        case OP_LTE:
        case OP_NEGATE:
        case OP_DEBUG:
        case OP_NOT:
        case OP_SUBSCRIPT:
        case OP_SUBS_ASSIGN: {
            return disasmSimpleIns(def.name, offset);
        }

        case OP_CONST:
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
        case OP_IMPORT:
        case OP_MODGET: {
            return disasmConstIns(def.name, offset, bt);
        }

        case OP_JUMP_IF_FALSE:
        case OP_JUMP:
        case OP_POP_JUMP_IF_FALSE:
        case OP_POP_JUMP_IF_TRUE: {
            return disasmJumpIns(def.name, offset, 1, bt);
        }

        case OP_LOOP: {
            return disasmJumpIns(def.name, offset, -1, bt);
        }

        case OP_SET_LOCAL:
        case OP_GET_LOCAL:
        case OP_SET_UPVAL:
        case OP_GET_UPVAL:
        case OP_CALL: {
            return disasmBytesIns(def.name, offset, bt);
        }
        case OP_MAP:
        case OP_ARRAY: {
            return disasmComplexDSIns(def.name, offset, bt);
        }
        case OP_CLOSURE: {
            u16 constant = ReadU16(bt, offset + 1);
            PanPrint("%s%s%s", TermGreen(), "OpClosure", TermReset());
            PanPrint("%4d ", constant);
            PrintValue(bt->constPool[constant]);
            PanPrint("\n");
            return offset + 3;
        }
    }
    return offset + 1;
}

void DebugBytecode(const PBytecode *bt, u64 offset) {
    PanPrint("==== DEBUG BYTECODE ====\n");

    u64 i = 0;
    while (i < bt->codeCount) {
        i = DisasmBytecode(bt, i);
    }

    PanPrint("====  END BYTECODE  ====\n");
}

static bool shouldRecordPos(PBytecode *b, Token *tok) {
    u64 tableCount = (u64)arrlen(b->posTable);
    if (tableCount == 0) {
        return true;
    } else {
        PBtPosInfo *last = &b->posTable[tableCount - 1];
        if (last->col != tok->col || last->line != tok->line ||
            last->len != tok->len) {
            return true;
        }
    }

    return false;
}

static bool recordPos(PBytecode *b, Token *tok) {
    bool shouldRecord = shouldRecordPos(b, tok);
    if (!shouldRecord) {
        return false;
    }

    PBtPosInfo entry = {
        .startOffset = b->codeCount,
        .line = tok->line,
        .col = tok->col,
        .len = tok->len,
        .gcol = tok->gcol,
        .glen = tok->glen,
        .token = tok
    };

    arrput(b->posTable, entry);
    return true;
}

u64 EmitBytecode(PBytecode *b, Token *tok, PanOpCode op) {
    if (tok != NULL) {
        recordPos(b, tok);
    }
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    return pos;
}

u64 EmitRawU16(PBytecode *b, u16 a) {
    u64 pos = b->codeCount;
    arrput(b->code, (u8)((a >> 8) & 0xFF));
    arrput(b->code, (u8)(a & 0xFF));
    b->codeCount += 2;
    return pos;
}

u64 EmitRawU8(PBytecode *b, u8 a) {
    u64 pos = b->codeCount;
    arrput(b->code, a);
    b->codeCount++;
    return pos;
}

u64 EmitBytecodeWithOneArg(PBytecode *b, Token *tok, PanOpCode op, u16 a) {
    if (tok != NULL) {
        recordPos(b, tok);
    }
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, (u8)((a >> 8) & 0xFF));
    b->codeCount++;
    arrput(b->code, (u8)(a & 0xFF));
    b->codeCount++;
    // arrput(b->tokens, tok);
    // arrput(b->lines, tok->line);
    return pos;
}
u64 EmitBytecodeWithTwoArgs(
    PBytecode *b, Token *tok, PanOpCode op, u8 one, u8 two
) {
    if (tok != NULL) {
        recordPos(b, tok);
    }
    u64 pos = b->codeCount;
    arrput(b->code, op);
    b->codeCount++;
    arrput(b->code, one);
    b->codeCount++;
    arrput(b->code, two);
    b->codeCount++;
    // arrput(b->tokens, tok);
    // arrput(b->lines, tok->line);
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
