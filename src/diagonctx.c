/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "include/diagonctx.h"
#include <stdarg.h>
#include <stdio.h>

void ReportDiag(PDiagonCtx *ctx, Token *token, PanDiagCode code, ...) {
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
