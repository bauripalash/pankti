#if defined(PANKTI_BUILD_DEBUG)

#include "include/flags.h"
#include "include/printer.h"
#include <stdbool.h>
#include <stdlib.h>

#define DEBUG_LEXER_ENV    "DEBUG_LEXER"
#define DEBUG_PARSER_ENV   "DEBUG_PARSER"
#define DEBUG_BYTECODE_ENV "DEBUG_BYTECODE"
#define DEBUG_TIMES_ENV    "DEBUG_TIMES"
#define DEBUG_GC_ENV       "DEBUG_GC"
#define STRESS_GC_ENV      "STRESS_GC"

PanDebugFlags panDebugFlags = {0};

static inline bool readEnvFlag(const char *name) {
    if (name == NULL) {
        return false;
    }
    const char *val = getenv(name);
    if (val == NULL) {
        return false;
    }

    return (val[0] == '1' || val[0] == 't' || val[0] == 'y');
}

void InitDebugFlags(void) {
    panDebugFlags.lexer = readEnvFlag(DEBUG_LEXER_ENV);
    panDebugFlags.parser = readEnvFlag(DEBUG_PARSER_ENV);
    panDebugFlags.bytecode = readEnvFlag(DEBUG_BYTECODE_ENV);
    panDebugFlags.times = readEnvFlag(DEBUG_TIMES_ENV);
    panDebugFlags.gc = readEnvFlag(DEBUG_GC_ENV);
    panDebugFlags.stressGc = readEnvFlag(STRESS_GC_ENV);
}

void PrintDebugFlags(void) {
    PanPrint("[DEBUG FLAGS]\n");
    PanPrint("\n");
    PanPrint("    DEBUG_LEXER    = %s\n", panDebugFlags.lexer ? "on" : "off");
    PanPrint("    DEBUG_PARSER   = %s\n", panDebugFlags.parser ? "on" : "off");
    PanPrint(
        "    DEBUG_BYTECODE = %s\n", panDebugFlags.bytecode ? "on" : "off"
    );
    PanPrint("    DEBUG_TIMES    = %s\n", panDebugFlags.times ? "on" : "off");
    PanPrint("    DEBUG_GC       = %s\n", panDebugFlags.gc ? "on" : "off");
    PanPrint(
        "    STRESS_GC      = %s\n", panDebugFlags.stressGc ? "on" : "off"
    );
}
#else
int _flags_(void) { return 0; }
#endif
