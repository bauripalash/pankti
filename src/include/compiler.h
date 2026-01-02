#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "object.h"
#include "opcode.h"
#include "ptypes.h"
#include "token.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_COMPILER_LOCAL_COUNT 65535

// Forward declaration for GC
typedef struct Pgc Pgc;

typedef struct PLocal {
    Token *name;
    int depth;
} PLocal;

// Pankti Compiler Object
typedef struct PCompiler {
    // Parser passed AST
    PStmt **prog;
    // How many top level statements are there in `prog`
    u64 progCount;

    // Bytecode Object
    PBytecode *code;

    // Local variables array
    PLocal locals[MAX_COMPILER_LOCAL_COUNT];
    // How many locals are currently there in locals array
    int localCount;
    // Current compiling scope depth
    // `0` means top level script
    // the more deep we the go more scopeDepth is increased
    int scopeDepth;

    // Pointer to garbage collector
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
