#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "gc.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"
#include "env.h"

typedef enum PModType {
    PMOD_STDLIB,
    PMOD_SCRIPT,
} PModType;

typedef struct PModule {
    PModType type;
    PEnv *env;
    char *pathname;
} PModule;

typedef struct ModProxyEntry {
    uint64_t key;
    char *name;
    PModule *mod;
} ModProxyEntry;

// The Interpreter Object
typedef struct PInterpreter {
    // Array of statements to execute
    PStmt **program;
    // Reference to `PanktiCore` object
    void *core;
    // The Parent Environment used across closures
    PEnv *env;
    ModProxyEntry *proxyTable;
    size_t proxyCount;
    PModule **mods;
    size_t modCount;
    Pgc *gc;
} PInterpreter;

// Create New Interpreter
// `prog` = Array of statements to execute. Given by parser
PInterpreter *NewInterpreter(Pgc *gc, PStmt **prog);

// Free Interpreter
void FreeInterpreter(PInterpreter *it);

// Execute the statements
void Interpret(PInterpreter *it);

#ifdef __cplusplus
}
#endif

#endif
