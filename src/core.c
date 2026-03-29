#include "include/core.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/compiler.h"
#include "include/errctx.h"
#include "include/flags.h"
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

static void coreRuntimeErrorBridge(
    void *ctx, Token *tok, const char *msg, bool fatal
) {
    CoreRuntimeError((PanktiCore *)ctx, tok, msg);
}

static void coreCompilerErrorBridge(
    void *ctx, Token *tok, const char *msg, bool fatal
) {
    CoreCompilerError((PanktiCore *)ctx, tok, msg);
}

static void coreParserErrorBridge(
    void *ctx, Token *tok, const char *msg, bool fatal
) {
    CoreParserError((PanktiCore *)ctx, tok, msg, fatal);
}

PanktiCore *NewCore(const char *scriptPath) {
    PanktiCore *core = PCreate(PanktiCore);
    if (core == NULL) {
        return NULL;
    }
    core->scriptPath = scriptPath;
    core->source = PanReadFile(core->scriptPath);
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

    core->scriptArgs = NULL;
    core->scriptArgCount = 0;

    core->caughtError = false;
    core->runtimeError = false;
    core->gc = NewGc();
    core->lexer->timestamp = core->gc->timestamp;
    core->compiler = NewCompiler(core->gc);
    core->vm = NewVm(core);
    GcRegisterRootMarker(core->gc, VmMarkRoots, core->vm);
    GcRegisterRootMarker(core->gc, CompilerMarkRoots, core->compiler);
    core->vm->errCtx =
        (PErrorCtx){.report = coreRuntimeErrorBridge, .ctx = core};

    core->compiler->errCtx =
        (PErrorCtx){.report = coreCompilerErrorBridge, .ctx = core};

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

    if (core->compiler != NULL) {
        FreeCompiler(core->compiler);
    }

    if (core->vm != NULL) {
        FreeVm(core->vm);
    }

    PFree(core);
}

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

#if defined(PANKTI_BUILD_DEBUG)
    clock_t start, end;

    if (FLAG_DEBUG_TIMES) {
        start = clock();
    }

#endif

    ScanTokens(core->lexer);

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        end = clock();
    }
    if (FLAG_DEBUG_LEXER) {
        PanPrint("==== Token ====\n");
        for (int i = 0; i < arrlen(core->lexer->tokens); i++) {
            PrintToken(core->lexer->tokens[i]);
            PanPrint("\n");
        }
        PanPrint("===============\n");
    }
    if (FLAG_DEBUG_TIMES) {
        double lexerTime = ((double)(end - start)) / CLOCKS_PER_SEC;
        PanPrint("[DEBUG] Lexer finished in : %f sec.\n", lexerTime);
    }
#endif
    if (core->lexer->hasError) {
        FreeCore(core);
        exit(1);
    }

    core->parser = NewParser(core->gc, core->lexer);
    core->parser->errCtx =
        (PErrorCtx){.report = coreParserErrorBridge, .ctx = core};

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        start = clock();
    }
#endif

    PStmt **prog = ParseParser(core->parser);

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        end = clock();
    }
    if (FLAG_DEBUG_PARSER) {
        PanPrint("===== AST =====\n");
        for (int i = 0; i < arrlen(prog); i++) {
            AstStmtPrint(prog[i], 0);
        }
        PanPrint("===== END =====\n");
    }
    if (FLAG_DEBUG_TIMES) {
        double parserTime = ((double)(end - start)) / CLOCKS_PER_SEC;
        PanPrint("[DEBUG] Parser finished in : %f sec.\n", parserTime);
    }
#endif

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

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        start = clock();
    }
#endif
    CompilerCompile(core->compiler, prog);

    PObj *comFn = GetCompiledFunction(core->compiler);

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        end = clock();
    }
    if (FLAG_DEBUG_BYTECODE) {
        DebugBytecode(comFn->v.OComFunction.code, 0);
    }

    if (FLAG_DEBUG_TIMES) {
        double compilerTime = ((double)(end - start)) / CLOCKS_PER_SEC;
        PanPrint("[DEBUG] Compiler finished in : %f sec.\n", compilerTime);
    }
#endif
    SetupVm(core->vm, core->gc, comFn);

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        start = clock();
    }
#endif
    VmRun(core->vm);
#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_TIMES) {
        end = clock();
        double vmTime = ((double)(end - start)) / CLOCKS_PER_SEC;
        PanPrint("[DEBUG] VM finished in : %f sec.\n", vmTime);
    }
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
    PanFPrint(
        stderr, "  %s%llu%s | ", TermBlue(), (unsigned long long)lineNum,
        TermReset()
    );

    // if there's something before the token in line, print it
    if (beforeTokLen > 0) {
        PanFPrint(stderr, "%.*s", (long)beforeTokLen, lineStart);
    }

    // Print the problematic token
    PanFPrint(
        stderr,
        "%s"
        "%.*s"
        "%s",
        TermErrorColorStart(), (long)tokLen, tokPtr, TermErrorColorEnd()
    );

    // If there's something after the token in line, print it
    if (afterTokLen) {
        PanFPrint(stderr, "%.*s", (long)afterTokLen, afterTokPtr);
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
