#include "include/core.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ansicolors.h"
#include "include/ast.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/token.h"
#include "include/utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

PanktiCore *NewCore(const char *path) {
    PanktiCore *core = PCreate(PanktiCore);
    core->path = path;
    core->source = ReadFile(core->path);
    if (core->source == NULL) {
        printf("Failed to Read Source Code\n");
        PFree(core);
        return NULL;
    }
    core->lexer = NewLexer(core->source);
    if (core->lexer == NULL) {
        PFree(core);
        return NULL;
    }
    core->lexer->core = core;
    core->parser = NULL;
    core->caughtError = false;
    core->runtimeError = false;
    core->it = NULL;
    core->gc = NewGc();
    core->lexer->timestamp = core->gc->timestamp;
    return core;
}

void FreeCore(PanktiCore *core) {
    if (core == NULL) {
        return;
    }
    if (core->gc != NULL) {
        FreeGc(core->gc);
    }
    if (core->parser != NULL) {
        FreeParser(core->parser);
    }

    if (core->lexer != NULL) {
        FreeLexer(core->lexer);
    }

    if (core->it != NULL) {
        FreeInterpreter(core->it);
    }

    PFree(core);
}

// Print All the Scanned Tokens
//#define DEBUG_LEXER
// Print the Ast
//#define DEBUG_PARSER
// Print time it takes to finish each step
//#define DEBUG_TIMES

void RunCore(PanktiCore *core) {
    if (core == NULL) {
        printf("Internal Error : Failed to create Pankti Core\n");
        exit(1);
    }
    if (core->lexer == NULL) {
        printf("Internal Error : Failed to create Pankti Lexer\n");
        exit(1);
    }
    stbds_rand_seed((size_t)time(NULL));
    core->lexer->core = core;
#if defined DEBUG_TIMES
    clock_t lxTic = clock();
#endif
    ScanTokens(core->lexer);
#if defined DEBUG_TIMES
    clock_t lxToc = clock();
    printf(
        "[DEBUG] Scanner finished : %f sec.\n",
        (double)(lxToc - lxTic) / CLOCKS_PER_SEC
    );
#endif
#if defined DEBUG_PARSER
    printf("==== Token ====\n");
    for (int i = 0; i < arrlen(core->lexer->tokens); i++) {
        PrintToken(core->lexer->tokens[i]);
        printf("\n");
    }
    printf("===============\n");
#endif
    if (core->lexer->hasError) {
        printf("Lexer Error found!\n");
		FreeCore(core);
		exit(1);
    }

    core->parser = NewParser(core->gc, core->lexer);
    core->parser->core = core;
#if defined DEBUG_TIMES
    clock_t pTic = clock();
#endif
    PStmt **prog = ParseParser(core->parser);
#if defined DEBUG_TIMES
    clock_t pToc = clock();
    printf(
        "[DEBUG] Parser finished : %f sec.\n",
        (double)(pToc - pTic) / CLOCKS_PER_SEC
    );
#endif
#if defined DEBUG_PARSER
    printf("===== AST =====\n");
    for (int i = 0; i < arrlen(prog); i++) {
        AstStmtPrint(prog[i], 0);
    }
    printf("===== END =====\n");
#endif
    if (core->parser->hasError) {
        printf("Parser Error(s) found!\n");

    }

	if (core->lexer->hasError || core->parser->hasError) {
	    FreeCore(core);
        exit(1);
	}

    core->it = NewInterpreter(core->gc, prog);
    core->it->core = core;
#if defined DEBUG_TIMES
    clock_t inTic = clock();
#endif

    Interpret(core->it);
#if defined DEBUG_TIMES
    clock_t inToc = clock();
    printf(
        "[DEBUG] Interpreter finished : %f sec.\n",
        (double)(inToc - inTic) / CLOCKS_PER_SEC
    );
#endif

    if (core->caughtError) {
        printf("Runtime Error found!\n");
        FreeCore(core);
        exit(1);
    }
}

static inline char * coreErrorToStr(PCoreErrorType errtype){
	switch (errtype) {
		case PCERR_LEXER:
		case PCERR_PARSER:
			return "Syntax";
		case PCERR_RUNTIME:
			return "Runtime";
	}

	return "Unknown";
}

// Simply print error message with line and column numbers, if present
static void printErrMsg(
    PanktiCore *core, size_t line, size_t col, const char *msg, bool hasPos, PCoreErrorType errtype
) {
    if (hasPos) {
        printf("[");

        if (line >= 1) {
            printf("line: %zu", line);
        }

        if (col >= 1) {
            printf(" : column: %zu", col);
        }
        printf("] ");
    }

    printf(TERMC_RED "%s Error: %s" TERMC_RESET "\n", coreErrorToStr(errtype), msg);
}

void CoreRuntimeError(PanktiCore * core, Token * token, const char * msg){
	size_t line = 0;
	size_t col = 0;
	if (token != NULL) {
		line = token->line;
		col = token->col;
	}

	printErrMsg(core, line, col, msg, token != NULL, PCERR_RUNTIME);
	printf("Runtime Error Occured!\n");
	FreeCore(core);
	exit(3);
}

void CoreParserError(PanktiCore * core, Token * token, const char * msg){
	size_t line = 0;
	size_t col = 0;
	if (token != NULL) {
		line = token->line;
		col = token->col;
	}

	printErrMsg(core, line, col, msg, token != NULL, PCERR_PARSER);
}

void CoreLexerError(
    PanktiCore *core, size_t line, size_t col, const char *msg
) {
    size_t _line = 0;
	size_t _col = 0;

	if (line != SIZE_MAX) {
		_line = line;
	}

	if (col != SIZE_MAX) {
		_col = col;
	}

	bool dontHavePos = line == SIZE_MAX || col == SIZE_MAX;

	printErrMsg(core, _line, _col, msg, !dontHavePos, PCERR_LEXER);
}
