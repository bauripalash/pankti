#include "include/core.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ansicolors.h"
#include "include/ast.h"
#include "include/compiler.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/lexer.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/parser.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/token.h"
#include "include/utils.h"
#include "include/vm.h"

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
        PanPrint("Failed to Read Source Code\n");
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
    // core->it = NULL;
    core->gc = NewGc();
    core->lexer->timestamp = core->gc->timestamp;
    core->compiler = NewCompiler(core);
    core->vm = NewVm(core);
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

    // if (core->it != NULL) {
    //     FreeInterpreter(core->it);
    // }

    if (core->compiler != NULL) {
        FreeCompiler(core->compiler);
    }

    if (core->vm != NULL) {
        FreeVm(core->vm);
    }

    PFree(core);
}

#ifdef PANKTI_BUILD_DEBUG
// Print All the Scanned Tokens
#define DEBUG_LEXER
// Print the Ast
#define DEBUG_PARSER
// Print time it takes to finish each step
#define DEBUG_TIMES

// Print Bytecode of Script
#define DEBUG_BYTECODE
#endif

PCoreErrorType RunCore(PanktiCore *core) {
    if (core == NULL) {
        PanPrint("Internal Error : Failed to create Pankti Core\n");
        return PCERR_CORE;
    }
    if (core->lexer == NULL) {
        PanPrint("Internal Error : Failed to create Pankti Lexer\n");
        return PCERR_CORE;
    }
    stbds_rand_seed((size_t)time(NULL));
    core->lexer->core = core;
#if defined DEBUG_TIMES
    clock_t lxTic = clock();
#endif
    ScanTokens(core->lexer);
#if defined DEBUG_TIMES
    clock_t lxToc = clock();
    PanPrint(
        "[DEBUG] Scanner finished : %f sec.\n",
        (double)(lxToc - lxTic) / CLOCKS_PER_SEC
    );
#endif
#if defined DEBUG_PARSER
    PanPrint("==== Token ====\n");
    for (int i = 0; i < arrlen(core->lexer->tokens); i++) {
        PrintToken(core->lexer->tokens[i]);
        PanPrint("\n");
    }
    PanPrint("===============\n");
#endif
    if (core->lexer->hasError) {
        PanPrint("Lexer Error found!\n");
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
    PanPrint(
        "[DEBUG] Parser finished : %f sec.\n",
        (double)(pToc - pTic) / CLOCKS_PER_SEC
    );
#endif
#if defined DEBUG_PARSER
    PanPrint("===== AST =====\n");
    for (int i = 0; i < arrlen(prog); i++) {
        AstStmtPrint(prog[i], 0);
    }
    PanPrint("===== END =====\n");
#endif
    if (core->parser->hasError) {
        PanPrint("Parser Error(s) found!\n");
    }

    if (core->lexer->hasError) {
        return PCERR_LEXER;
    }

    if (core->parser->hasError) {
        return PCERR_PARSER;
    }

    if (core->lexer->hasError || core->parser->hasError) {
        FreeCore(core);
        exit(1);
    }

    /*
    char *useCompiler = getenv("PAN_COMPILER");
    if (useCompiler == NULL) {

        core->it = NewInterpreter(core->gc, prog);
        core->it->core = core;
#if defined DEBUG_TIMES
        clock_t inTic = clock();
#endif

        Interpret(core->it);
#if defined DEBUG_TIMES
        clock_t inToc = clock();
        PanPrint(
            "[DEBUG] Interpreter finished : %f sec.\n",
            (double)(inToc - inTic) / CLOCKS_PER_SEC
        );
#endif

    } else {
*/

#if defined(DEBUG_TIMES)
    clock_t cmpTic = clock();
#endif
    CompilerCompile(core->compiler, prog);
#if defined(DEBUG_TIMES)
    clock_t cmpToc = clock();
    PanPrint(
        "[DEBUG] Compiler finished : %f sec.\n",
        (double)(cmpToc - cmpTic) / CLOCKS_PER_SEC
    );
#endif
    PObj *comFn = GetCompiledFunction(core->compiler);

#if defined(DEBUG_BYTECODE)
    DebugBytecode(comFn->v.OComFunction.code, 0);
#endif
    SetupVm(core->vm, core->gc, comFn);
#if defined(DEBUG_TIMES)
    clock_t vmTic = clock();
#endif
    VmRun(core->vm);
#if defined(DEBUG_TIMES)
    clock_t vmToc = clock();
    PanPrint(
        "[DEBUG] VM finished : %f sec.\n",
        (double)(vmToc - vmTic) / CLOCKS_PER_SEC
    );
#endif

    if (core->caughtError) {
        PanPrint("Runtime Error found!\n");
        FreeCore(core);
        exit(1);
        return PCERR_RUNTIME;
    }

    return PCERR_OK;
}

static inline char *coreErrorToStr(PCoreErrorType errtype) {
    switch (errtype) {
        case PCERR_LEXER:
        case PCERR_PARSER: return "Syntax";
        case PCERR_RUNTIME: return "Runtime";
        case PCERR_COMPILER: return "Compiler";
        case PCERR_CORE: return "Initialization";
        case PCERR_OK: return "";
    }

    return "Unknown";
}

// Simply print error message with line and column numbers, if present
static void printErrMsg(
    PanktiCore *core,
    u64 line,
    u64 col,
    const char *msg,
    bool hasPos,
    PCoreErrorType errtype
) {
    if (hasPos) {
        PanPrint("[");

        if (line >= 1) {
            PanPrint("line: %zu", line);
        }

        if (col >= 1) {
            PanPrint(" : column: %zu", col);
        }
        PanPrint("] ");
    }

    PanPrint(
        TERMC_RED "%s Error: %s" TERMC_RESET "\n", coreErrorToStr(errtype), msg
    );
}

void CoreRuntimeError(PanktiCore *core, Token *token, const char *msg) {
    u64 line = 0;
    u64 col = 0;
    if (token != NULL) {
        line = token->line;
        col = token->col;
    }

    printErrMsg(core, line, col, msg, token != NULL, PCERR_RUNTIME);
    PanPrint("Runtime Error Occured!\n");
    FreeCore(core);
    exit(EXIT_FAILURE);
}

void CoreParserError(PanktiCore *core, Token *token, const char *msg) {
    u64 line = 0;
    u64 col = 0;
    if (token != NULL) {
        line = token->line;
        col = token->col;
    }

    printErrMsg(core, line, col, msg, token != NULL, PCERR_PARSER);
}

void CoreLexerError(PanktiCore *core, u64 line, u64 col, const char *msg) {
    u64 _line = 0;
    u64 _col = 0;

    if (line != UINT64_MAX) {
        _line = line;
    }

    if (col != UINT64_MAX) {
        _col = col;
    }

    bool dontHavePos = line == UINT64_MAX || col == UINT64_MAX;

    printErrMsg(core, _line, _col, msg, !dontHavePos, PCERR_LEXER);
}

void CoreCompilerError(PanktiCore *core, Token *token, const char *msg) {
    u64 line = 0;
    u64 col = 0;
    if (token != NULL) {
        line = token->line;
        col = token->col;
    }

    printErrMsg(core, line, col, msg, token != NULL, PCERR_COMPILER);
    PanPrint("Compiler Error Occured!\n");
    FreeCore(core);
    exit(EXIT_FAILURE);
}
