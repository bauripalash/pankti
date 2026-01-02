#ifndef OPCODE_H
#define OPCODE_H

#include "object.h"
#include "ptypes.h"
#include "token.h"
#ifdef __cplusplus
extern "C" {
#endif

// How many Constants can be in the Bytecode
#define MAX_CONST_COUNT 65535

// Pankti Opcodes
typedef enum PanOpCode {
    // Make Constant
    OP_CONST = 0,
    // Temporary debug statment
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
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
} PanOpCode;

// OpCode definition
typedef struct POpDefinition {
    // Name of the opcode
    char name[256];
    // how many operands will be there
    u8 operands;
    // how many u8 bytes will each operands take
    // first item in the array will define the size of first operand..
    u8 operandWidths[8];
} POpDefinition;

// Get name of the opcode
const char *OpCodeToStr(PanOpCode code);
// Get definition of the opcocde
POpDefinition GetOpDefinition(PanOpCode code);

// Bytecode Object
typedef struct PBytecode {
    // Raw bytes
    u8 *code;
    // How many bytes are there
    u64 codeCount;
    // Constants list
    PValue *constPool;
    // How many Constants are there
    u16 constCount;
    // Token array with each array being linked for the each opcode
    // `tokens` count will be equal to how many opcodes are there
    Token **tokens;
    // Direct link to token's line number
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

// Read a u16 operands from raw code bytes
u16 ReadU16RawCode(const u8 *code, u64 offset);

#ifdef __cplusplus
}
#endif

#endif
