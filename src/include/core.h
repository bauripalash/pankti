#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gc.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "ptypes.h"
#include "token.h"
#include <stddef.h>

// Main Body for whole pankti runtime
typedef struct PanktiCore {
    // Lexer
    Lexer *lexer;
    // Parser
    Parser *parser;
    // Interpreter
    PInterpreter *it;

    Pgc *gc;

    // Original script as is
    char *source;
    // Path to script
    const char *path;

    // Has error?
    bool caughtError;
    // Has runtime error?
    bool runtimeError;
} PanktiCore;

typedef enum PCoreErrorType {
    PCERR_RUNTIME = 0,
    PCERR_LEXER = 1,
    PCERR_PARSER = 2,
} PCoreErrorType;

// Create New Pankti Core
// `path` = File path of the script
PanktiCore *NewCore(const char *path);
// Free everything Pankti Core allocated
void FreeCore(PanktiCore *core);
// Run the script
void RunCore(PanktiCore *core);
// Throw Runtime Error and Quit
// `core` = Interpreter Core
// `token` = Token where the runtime error occurred; can be NULL if something
// out of the world or something ambiguous happened.
// `msg` = Error Message
void CoreRuntimeError(PanktiCore *core, Token *token, const char *msg);

// Throw Parser/Syntax Error and Continue parsing the code.
// `core` = Interpreter Core
// `token` = Token where the runtime error occurred; can be NULL if something
// out of the world or something ambiguous happened.
// `msg` = Error Message
void CoreParserError(PanktiCore *core, Token *token, const char *msg);

// Throw Lexer/Syntax Error and Continue scanning for tokens to catch as many as
// errors as we can find.
// `core` = Interpreter Core
// `line` = Non negative line number. If ambiguous pass U64_MAX.
// `col` = Non negative byte based column number. If ambiguous pass U64_MAX.
// `msg` = Error Message
void CoreLexerError(PanktiCore *core, u64 line, u64 col, const char *msg);
#ifdef __cplusplus
}
#endif
#endif
