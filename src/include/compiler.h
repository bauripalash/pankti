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

// Maximum number of locals compiler can hold
// Maximum range of u16
#define MAX_COMPILER_LOCAL_COUNT 65535

// Forward declaration for GC
typedef struct Pgc Pgc;
typedef struct PanktiCore PanktiCore;

// Local Variable Structure
typedef struct PLocal {
    Token *name;
    int depth;
} PLocal;

// Compiled Function Type
typedef enum PCompFuncType {
    // Plain Used Defined Function
    COMP_FN_FUNCTION,
    // Top Lever Script
    COMP_FN_SCRIPT,
} PCompFuncType;

// Pankti Compiler Object
typedef struct PCompiler {
    // Parser passed AST
    PStmt **prog;
    // How many top level statements are there in `prog`
    u64 progCount;

    // Local variables array
    PLocal locals[MAX_COMPILER_LOCAL_COUNT];
    // How many locals are currently there in locals array
    int localCount;
    // Current compiling scope depth
    // `0` means top level script
    // the more deep we the go more scopeDepth is increased
    int scopeDepth;

    Token *dummyToken;

    // Pointer to garbage collector
    Pgc *gc;
    // Core
    PanktiCore *core;
    // Compiled function object, where the bytecodes, constants will be
    // emitted. The function will be returned after compiling finished
    PObj *func;
    // Current compiling function type.
    // Top level script or user defined function
    PCompFuncType funcType;
    // Enclosing compiler
    struct PCompiler *enclosing;
} PCompiler;

// Create a new compiler object
PCompiler *NewCompiler(PanktiCore *core);
PCompiler *NewEnclosedCompiler(
    PanktiCore *core, PCompiler *comp, PCompFuncType ftype, Token *name
);

// Free the compiler
void FreeCompiler(PCompiler *comp);

// Compile a root AST Node
bool CompilerCompile(PCompiler *compiler, PStmt **prog);

// Get just compiled function which has all the bytecode and constants
PObj *GetCompiledFunction(PCompiler *comp);

#ifdef __cplusplus
}
#endif

#endif
