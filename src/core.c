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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

PanktiCore *NewCore(const char *path) {
    PanktiCore *core = PCreate(PanktiCore);
    core->path = path;
    core->source = ReadFile(core->path);
    core->lexer = NewLexer(core->source);
    core->lexer->core = core;
    core->parser = NULL;
    core->caughtError = false;
    core->runtimeError = false;
    core->it = NULL;
    core->gc = NewGc();
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
static bool DebugLexer = true;
// Print the Ast
static bool DebugParser = true;

#define DEBUG_TIMES

void RunCore(PanktiCore *core) {
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
    if (DebugLexer) {
        printf("==== Token ====\n");
        for (int i = 0; i < arrlen(core->lexer->tokens); i++) {
            PrintToken(core->lexer->tokens[i]);
            printf("\n");
        }
        printf("===============\n");
    }

    if (core->caughtError) {
        printf("Lexer Error found!\n");
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

    if (core->caughtError) {
        printf("Parser Error found!\n");
        // exit(1);
    }
    if (DebugParser) {
        printf("===== AST =====\n");
        for (int i = 0; i < arrlen(prog); i++) {
            AstStmtPrint(prog[i], 0);
        }
        printf("===== END =====\n");
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
        exit(1);
    }
}

static void reportError(PanktiCore *core, int line, const char *msg) {
	if (line > 0) {
		int lineIndex = line - 1;
		int lineCount = 0;
		char ** lines = StrSplit(core->lexer->source, '\n', &lineCount);
		if (line < lineCount) {
			printf("%d | %s",line ,lines[lineIndex]);
			printf("<--\n");
		}
	}
    printf("[%d] Error : %s\n", line, msg);
    core->caughtError = true;
}

static void reportRuntimeError(PanktiCore * core, Token * tok){

}

static void printErrMsg(PanktiCore * core, int line, int col, const char *msg){
	printf("[");
	if (line >= 1) {
		printf("line: %d", line);
	}
	if (col >= 1) {
		printf(" : column: %d", col);
	}
	printf("] ");
	printf(TERMC_RED "Error: %s" TERMC_RESET "\n", msg);
}

void CoreError(PanktiCore *core, Token * token, const char *msg) {
    //reportError(core, token == NULL ? -1 : token->line, msg);
	int line = -1;
	int col = -1;
	if (token != NULL) {
		line = token->line;
		col = token->col;
	}
	printErrMsg(core, line, col, msg);
	
}

void CoreLexerError(PanktiCore * core, long line, long col, const char * msg){
	reportError(core, line, msg);
}
