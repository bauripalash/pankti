#include "include/argparse.h"
#include "include/printer.h"
#include "include/version.h"
#include <stdbool.h>
#include <stdio.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "external/optparse/optparse.h"

#if defined(PANKTI_BUILD_DEBUG)
#include "include/flags.h"
#define PANKTI_SHORT_ARGS "hvLPBTGS"
#else
#define PANKTI_SHORT_ARGS "hv"
#endif

static const struct optparse_long PANKTI_LONG_OPTS[] = {
    {"help", 'h', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},

#if defined(PANKTI_BUILD_DEBUG)
    {"debug-lexer", 'L', OPTPARSE_NONE},
    {"debug-parser", 'P', OPTPARSE_NONE},
    {"debug-bytecode", 'B', OPTPARSE_NONE},
    {"debug-times", 'T', OPTPARSE_NONE},
    {"debug-gc", 'G', OPTPARSE_NONE},
    {"stress-gc", 'S', OPTPARSE_NONE},
#endif

    {0}
};

PanArgsResult ParsePanArgs(int argc, char **argv, PanktiArgs *out) {

    if (argc < 2 || argv == NULL || out == NULL) {
        PrintPanktiHelp();
        return PARGS_EXIT_ERR;
    }

    out->evalCode = NULL;
    out->scriptPath = NULL;
    out->scriptArgs = NULL;
    out->scriptArgCount = 0;

    const struct optparse_long *longopts = PANKTI_LONG_OPTS;

    struct optparse opts;
    optparse_init(&opts, argv);
    opts.permute = 0;

    int opt;
    while ((opt = optparse_long(&opts, longopts, NULL)) != -1) {
        switch (opt) {
            case 'h': {
                PrintPanktiHelp();
                return PARGS_EXIT_OK;
            }

            case 'v': {
                PrintPanktiVersion();
                return PARGS_EXIT_OK;
            }

#if defined(PANKTI_BUILD_DEBUG)

            case 'L': {
                panDebugFlags.lexer = true;
                break;
            }

            case 'P': {
                panDebugFlags.parser = true;
                break;
            }

            case 'B': {
                panDebugFlags.bytecode = true;
                break;
            }

            case 'T': {
                panDebugFlags.times = true;
                break;
            }

            case 'G': {
                panDebugFlags.gc = true;
                break;
            }

            case 'S': {
                panDebugFlags.stressGc = true;
                break;
            }
#endif

            case '?': {
                PanFPrint(stderr, "Invalid Flag");
                PrintPanktiHelp();
                return PARGS_EXIT_ERR;
            }
        }
    }

    char *scriptArg = optparse_arg(&opts);
    if (scriptArg != NULL) {
        out->scriptPath = scriptArg;

        int remaining = argc - opts.optind;
        if (remaining > 0) {
            out->scriptArgs = argv + opts.optind;
            out->scriptArgCount = remaining;
        }
    }

    if (out->scriptPath == NULL) {
        PrintPanktiHelp();
        return PARGS_EXIT_OK;
    }

    return PARGS_OK;
}

void PrintPanktiVersion(void) {
    PanPrint("Pankti Programming Language %s\n", PANKTI_VERSION);
}

static const char *HelpInfo =
    "Pankti Programming Language %s\n\n"
    "Usage:\n"
    "   pankti [options] [script.pn] [-- script-args]\n\n"
    "Options:\n"
    "   -h, --help              Show this help message\n"
    "   -v, --version           Show version information\n\n"
    "Examples:\n"
    "   pankti script.pn\n"
    "   pankti --version\n"
#if defined(PANKTI_BUILD_DEBUG)
    "\n"
    "Debug Options (debug builds only):\n"
    "   -L, --debug-lexer       Print Scanned Tokens\n"
    "   -P, --debug-parser      Print Parsed AST\n"
    "   -B, --debug-bytecode    Print Compiled Bytecode\n"
    "   -T, --debug-times       Print Times for Lexing, Parsing...etc.\n"
    "   -G, --debug-gc          Print Debug Events\n"
    "   -S, --stress-gc         Stress GC to to run on every allocation\n"
    "\n"
    "Environment Variables:\n"
    "(Debugging features can be turned on with environment variables)\n"
    "   DEBUG_LEXER=1   DEBUG_PARSER=1  DEBUG_BYTECODE=1\n"
    "   DEBUG_TIMES=1   DEBUG_GC=1      STRESS_GC=1\n"
#endif
    ;

void PrintPanktiHelp(void) { PanPrint(HelpInfo, PANKTI_VERSION); }
