#include "include/core.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
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

    free(core);
}

static bool DebugLexer = false;
static bool DebugParser = false;

void RunCore(PanktiCore *core) {
    core->lexer->core = core;
    ScanTokens(core->lexer);
    if (DebugLexer) {
        for (int i = 0; i < arrlen(core->lexer->tokens); i++) {
            PrintToken(core->lexer->tokens[i]);
        }
    }

    if (core->caughtError) {
        printf("Lexer Error found!\n");
        exit(1);
    }

    core->parser = NewParser(core->gc, core->lexer);
    core->parser->core = core;

    PStmt **prog = ParseParser(core->parser);

    if (core->caughtError) {
        printf("Parser Error found!\n");
        // exit(1);
    }
    if (DebugParser) {
        printf("==== AST ====\n");
        for (int i = 0; i < arrlen(prog); i++) {
            AstStmtPrint(prog[i], 0);
        }
        printf("==== END ====\n");
    }
    core->it = NewInterpreter(core->gc, prog);
    core->it->core = core;
    Interpret(core->it);
    if (core->caughtError) {
        printf("Runtime Error found!\n");
        exit(1);
    }
}

static void reportError(PanktiCore *core, int line, char *msg) {
    printf("[%d] Error : %s\n", line, msg);
    core->caughtError = true;
}

void CoreError(PanktiCore *core, int line, char *msg) {
    reportError(core, line, msg);
}
