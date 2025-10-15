#ifndef CORE_H
#define CORE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

// Main Body for whole interpreter program
typedef struct PanktiCore{
	// Lexer
	Lexer * lexer;
	// Parser
	Parser * parser;
	// Interpreter
	PInterpreter * it;
	// Original script as is
	char * source;
	// Path to script
	const char * path;

	// Has error?
	bool caughtError;
	// Has runtime error?
	bool runtimeError;
}PanktiCore;


// Create New Pankti Core
// `path` = File path of the script
PanktiCore * NewCore(const char * path);
// Free everything Pankti Core allocated
void FreeCore(PanktiCore * core);
// Run the script
void RunCore(PanktiCore * core);
// Show error
void CoreError(PanktiCore * core, int line, char * msg);
#ifdef __cplusplus
}
#endif
#endif
