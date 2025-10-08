#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "env.h"
typedef struct PInterpreter{
	PStmt ** program;
	void * core;
	PEnv * env;
}PInterpreter;

PInterpreter * NewInterpreter(PStmt ** prog);
void FreeInterpreter(PInterpreter * it);
void Interpret(PInterpreter * it);

#endif
