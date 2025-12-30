#ifndef OPCODE_H
#define OPCODE_H

#include "object.h"
#include "ptypes.h"
#include "token.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONST_COUNT 65535

typedef enum PanOpCode {
    OP_CONST = 0,
    OP_DEBUG,
    OP_RETURN,
    OP_TRUE,
    OP_FALSE,
    OP_NIL,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EXPONENT,
    OP_EQUAL,
    OP_NOTEQUAL,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_NEGATE,
    OP_NOT,
    OP_ARRAY,
    OP_MAP,
} PanOpCode;

typedef struct POpDefinition {
    char name[256];
    u8 operands;
    u8 operandWidths[8];
} POpDefinition;

const char *OpCodeToStr(PanOpCode code);
POpDefinition GetOpDefinition(PanOpCode code);

typedef struct PBytecode {
    u8 *code;
    u64 codeCount;
    PValue *constPool;
    u16 constCount;
    Token **tokens;
    u64 *lines;
} PBytecode;

// Create a new Bytecode Object
PBytecode *NewBytecode(void);

// Free Bytecode object
void FreeBytecode(PBytecode *b);

// Debug and Print Instructions in Bytecode
void DebugBytecode(const PBytecode *bt, u64 offset);

// Emit and Write a Opcode
u64 EmitBytecode(PBytecode *b, Token *tok, PanOpCode op);
// Emit and Write a Opcode with u16 operand
u64 EmitBytecodeWithOneArg(PBytecode *b, Token *tok, PanOpCode op, u16 a);
// Emit and Write a Opcode with two u8 operands
u64 EmitBytecodeWithTwoArgs(
    PBytecode *b, Token *tok, PanOpCode op, u8 one, u8 two
);

// Add New Constant to Constant pool and return its index
u16 AddConstantToPool(PBytecode *b, PValue value);

// Read a u16 operand from Bytecode
u16 ReadU16(const PBytecode *b, u64 offset);

u16 ReadU16RawCode(const u8 *code, u64 offset);

#ifdef __cplusplus
}
#endif

#endif
