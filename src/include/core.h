#ifndef CORE_H
#define CORE_H

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

typedef struct PanktiCore{
	Lexer * lexer;
	Parser * parser;
	PInterpreter * it;
	char * source;
	const char * path;

	bool caughtError;
	bool runtimeError;
}PanktiCore;


PanktiCore * NewCore(const char * path);
void FreeCore(PanktiCore * core);
void RunCore(PanktiCore * core);
void CoreError(PanktiCore * core, int line, char * msg);
#endif
