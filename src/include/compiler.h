#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "object.h"
#include "opcode.h"
#include "ptypes.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Pgc Pgc;

// Pankti Compiler Object
typedef struct PCompiler {
    PStmt **prog;
    u64 progCount;

    PBytecode *code;

    Pgc *gc;
} PCompiler;

// Create a new compiler object
PCompiler *NewCompiler(Pgc *gc);

// Free the compiler
void FreeCompiler(PCompiler *comp);

// Compile a root AST Node
bool CompilerCompile(PCompiler *compiler, PStmt **prog);

#ifdef __cplusplus
}
#endif

#endif
