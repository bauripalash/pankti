#ifndef PANKTI_DIAGON_CTX_H
#define PANKTI_DIAGON_CTX_H

#include "../gen/diagon.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

// Diagnostics reporting function for CoreXXError type functions
typedef void (*PDiagonReportFn)(
    void *ctx, Token *tok, PanDiagCode code, va_list args
);

// Diagnostics Context To call invoke diagnostic functions without
// having direct link to core
typedef struct PDiagonCtx {
    // Error reporting function
    PDiagonReportFn report;
    // Link to core
    void *ctx;
} PDiagonCtx;

void ReportDiag(PDiagonCtx *ctx, Token *token, PanDiagCode code);
void ReportDiagF(PDiagonCtx *ctx, Token *token, PanDiagCode code, ...);
void ReportDiagV(PDiagonCtx *ctx, Token * token, PanDiagCode code, va_list args);

#ifdef __cplusplus
}
#endif

#endif
