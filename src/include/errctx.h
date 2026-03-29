#ifndef PANKTI_ERROR_CTX_H
#define PANKTI_ERROR_CTX_H

#include "token.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// Error reporting function for CoreXXError type functions
typedef void (*ErrorReportFn)(
    void *ctx, Token *tok, const char *msg, bool fatal
);

// Error Context To call CoreXXError type error functions without
// having direct link to core
typedef struct PErrorCtx {
    // Error reporting function
    ErrorReportFn report;
    // Link to core
    void *ctx;
} PErrorCtx;

#ifdef __cplusplus
}
#endif

#endif
