#ifndef PANKTI_DIAGON_CTX_H
#define PANKTI_DIAGON_CTX_H

#include "token.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// Diagnostics reporting function for CoreXXError type functions
typedef void (*PDiagonReportFn)(
    void *ctx, Token *tok, const char *msg, bool fatal
);

// Diagnostics Context To call invoke diagnostic functions without
// having direct link to core
typedef struct PDiagonCtx {
    // Error reporting function
    PDiagonReportFn report;
    // Link to core
    void *ctx;
} PDiagonCtx;

#ifdef __cplusplus
}
#endif

#endif
