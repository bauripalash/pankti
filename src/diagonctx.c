#include "include/diagonctx.h"
#include <stdarg.h>
#include <stdio.h>

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
