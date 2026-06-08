/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "diagon.h"
#include <stdbool.h>
#include <stddef.h>
static const PanDiagInfo diagList[PANDIAG_CODE_COUNT] = {
//REPLACEME
};

const PanDiagInfo *DiagGetInfo(PanDiagCode code) {
    if (code < 0 || code >= PANDIAG_CODE_COUNT) {
        return NULL;
    }

    return &diagList[code];
}
const char *DiagGetMsg(PanDiagCode code) { return DiagGetInfo(code)->msg; }

const char *DiagGetHint(PanDiagCode code) {
    const PanDiagInfo *info = DiagGetInfo(code);
    if (info && info->hinted && info->hint[0] != '\0') {
        return info->hint;
    } else {
        return NULL;
    }
}
