#include "include/argparse.h"
#include "include/flags.h"
#include "include/printer.h"
#include "include/version.h"
#include <stdbool.h>
#include <stdio.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "external/optparse/optparse.h"

#if defined(PANKTI_BUILD_DEBUG)
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
        return PARGS_EXIT_ERR;
    }

    out->evalCode = NULL;
    out->scriptPath = NULL;
    out->scriptArgs = NULL;
    out->scriptArgCount = 0;

    const struct optparse_long *longopts = PANKTI_LONG_OPTS;

    struct optparse opts;
    optparse_init(&opts, argv);

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

    char *arg = optparse_arg(&opts);
    if (arg != NULL) {
        out->scriptPath = arg;
    }

    int remaining = argc - opts.optind;
    if (remaining > 0) {
        out->scriptArgs = argv + opts.optind;
        out->scriptArgCount = remaining;
    }

    if (out->scriptPath == NULL) {
        PrintPanktiHelp();
        return PARGS_EXIT_OK;
    }

    return PARGS_OK;
}

void PrintPanktiVersion(void) { PanFPrint(stdout, "%s\n", PANKTI_VERSION); }
void PrintPanktiHelp(void) {
    PanFPrint(stdout, "Pankti %s\n", "TODO_HELP_PANKTI");
}
