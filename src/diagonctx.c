#include "include/diagonctx.h"
#include <stdarg.h>
#include <stdio.h>

// BUG: empty va_list, UB?
void ReportDiag(PDiagonCtx *ctx, Token *token, PanDiagCode code) {
    va_list empty;
    ctx->report(ctx->ctx, token, code, empty);
}
void ReportDiagF(PDiagonCtx *ctx, Token *token, PanDiagCode code, ...) {
    va_list args;
    va_start(args, code);
    ctx->report(ctx->ctx, token, code, args);
    va_end(args);
}

void ReportDiagV(
    PDiagonCtx *ctx, Token *token, PanDiagCode code, va_list args
) {
    ctx->report(ctx->ctx, token, code, args);
}
