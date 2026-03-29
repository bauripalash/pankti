#ifndef PANKTI_ERROR_CTX_H
#define PANKTI_ERROR_CTX_H

#include "token.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ErrorReportFn)(
    void *ctx, Token *tok, const char *msg, bool fatal
);
typedef struct PErrorCtx {
    ErrorReportFn report;
    void *ctx;
} PErrorCtx;

#ifdef __cplusplus
}
#endif

#endif
