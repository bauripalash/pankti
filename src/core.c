#include "include/core.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/compiler.h"
#include "include/gc.h"
#include "include/lexer.h"
#include "include/object.h"
#include "include/parser.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/terminal.h"
#include "include/token.h"
#include "include/utils.h"
#include "include/vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(PANKTI_BUILD_DEBUG)
#include "include/opcode.h"
#endif

PanktiCore *NewCore(const char *path) {
    PanktiCore *core = PCreate(PanktiCore);
    if (core == NULL) {
        return NULL;
    }
    core->path = path;
    core->source = PanReadFile(core->path);
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
    srand(time(NULL));
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

static void printSourceLine(PanktiCore *core, u64 lineNum, u64 col, u64 len) {
    if (core == NULL || core->source == NULL || lineNum == 0) {
        return;
    }

    const char *ptr = core->source;
    u64 cur = 1; // current line number

    // as long we get character while current line is less than lineNum
    // advance the pointer and also increase line number on '\n'
    while (*ptr != '\0' && cur < lineNum) {
        if (*ptr == '\n') {
            cur++;
        }
        ptr++;
    }

    const char *lineStart = ptr;
    const char *lineEnd = ptr;

    // Fetch the line ending position
    while (*lineEnd != '\0' && *lineEnd != '\n') {
        lineEnd++;
    }

    // length of target line
    u64 lineLen = (u64)(lineEnd - lineStart);

    // get column index from column number (1 based column to 0 based column)
    u64 tokStart = (col > 0) ? (col - 1) : 0;

    // Safety check if token start went past line
    if (tokStart > lineLen) {
        tokStart = lineLen;
    }
    // token length
    u64 tokLen = len > 0 ? len : 1;

    // safety check if token length makes the token
    // position go beyond line limit
    if (tokStart + tokLen > lineLen) {
        tokLen = (lineLen > tokStart) ? (lineLen - tokStart) : 1;
    }

    // hello token bye
    //       ^ --> tokPtr
    const char *tokPtr = lineStart + tokStart;
    // hello token bye
    //            ^ --> afterTokPtr
    const char *afterTokPtr = tokPtr + tokLen;

    // hello token bye
    //|~~~~~| <-- beforeTokLen (length)
    u64 beforeTokLen = tokStart;

    // hello token bye
    //            |~~| <-- afterTokLen (length)
    u64 afterTokLen = (u64)(lineEnd - afterTokPtr);

    // Print the line number
    PanFPrint(stderr, "  %s%zu%s | ", TermBlue(), lineNum, TermReset());

    // if there's something before the token in line, print it
    if (beforeTokLen > 0) {
        PanFPrint(stderr, "%.*s", beforeTokLen, lineStart);
    }

    // Print the problematic token
    PanFPrint(
        stderr,
        "%s"
        "%.*s"
        "%s",
        TermErrorColorStart(), tokLen, tokPtr, TermErrorColorEnd()
    );

    // If there's something after the token in line, print it
    if (afterTokLen) {
        PanFPrint(stderr, "%.*s", afterTokLen, afterTokPtr);
    }

    PanFPrint(stderr, "\n");
}

// Simply print error message with line and column numbers, if present
static void printErrMsg(
    PanktiCore *core, u64 line, u64 col, const char *msg, PCoreErrorType errtype
) {
    PanFPrint(stderr, "\n");
    PanFPrint(
        stderr, "%s%s Error: %s%s\n", TermRed(), coreErrorToStr(errtype), msg,
        TermReset()
    );
}

void CoreRuntimeError(PanktiCore *core, Token *token, const char *msg) {
    u64 line = 0;
    u64 col = 0;
    u64 gcol = 0;
    u64 len = 0;

    if (token != NULL) {
        line = token->line;
        col = token->col;
        gcol = token->gcol;
        len = token->len;
    }

    printErrMsg(core, line, gcol, msg, PCERR_RUNTIME);
    printSourceLine(core, line, col, len);
    PanFPrint(stderr, "\n");
    VmPrintStackTrace(core->vm);
    FreeCore(core);
    exit(EXIT_FAILURE);
}

void CoreParserError(
    PanktiCore *core, Token *token, const char *msg, bool fatal
) {
    u64 line = 0;
    u64 col = 0;
    u64 gcol = 0;
    u64 len = 0;

    if (token != NULL) {
        line = token->line;
        col = token->col;
        gcol = token->gcol;
        len = token->len;
    }

    printErrMsg(core, line, gcol, msg, PCERR_PARSER);
    printSourceLine(core, line, col, len);
    if (fatal) {
        FreeCore(core);
        exit(EXIT_FAILURE);
    }
}

void CoreLexerError(
    PanktiCore *core, u64 line, u64 col, u64 len, const char *msg
) {
    u64 _line = (line != UINT64_MAX) ? line : 0;
    u64 _col = (col != UINT64_MAX) ? col : 0;
    u64 _len = (len > 0) ? len : 1;
    bool hasPos = (line != UINT64_MAX && col != UINT64_MAX);

    printErrMsg(core, _line, _col, msg, PCERR_LEXER);

    if (hasPos && _line >= 1) {
        printSourceLine(core, _line, _col, _len);
    }
}

void CoreCompilerError(PanktiCore *core, Token *token, const char *msg) {
    u64 line = 0;
    u64 col = 0;
    u64 gcol = 0;
    u64 len = 0;

    if (token != NULL) {
        line = token->line;
        col = token->col;
        gcol = token->gcol;
        len = token->len;
    }

    printErrMsg(core, line, gcol, msg, PCERR_COMPILER);
    printSourceLine(core, line, col, len);

    FreeCore(core);
    exit(EXIT_FAILURE);
}
