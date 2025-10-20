#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "gc.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"
#include "env.h"

// The Interpreter Object
typedef struct PInterpreter {
    // Array of statements to execute
    PStmt **program;
    // Reference to `PanktiCore` object
    void *core;
    // The Parent Environment used across closures
    PEnv *env;

	Pgc * gc;
} PInterpreter;

// Create New Interpreter
// `prog` = Array of statements to execute. Given by parser
PInterpreter *NewInterpreter(Pgc * gc, PStmt **prog);

// Free Interpreter
void FreeInterpreter(PInterpreter *it);

// Execute the statements
void Interpret(PInterpreter *it);

#ifdef __cplusplus
}
#endif

#endif
