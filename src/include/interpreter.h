#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
typedef struct PInterpreter{
	PExpr * program;
}PInterpreter;

PInterpreter * NewInterpreter(PExpr * prog);
void FreeInterpreter(PInterpreter * it);
void Interpret(PInterpreter * it);

#endif
