#include "diagon.h"
#include <stdbool.h>
#include <stddef.h>
static const PanDiagInfo diagList[PANDIAG_CODE_COUNT] = {
//REPLACEME
};

const PanDiagInfo *DiagGetInfo(PanDiagCode code) {
    if (code < 0 || code > PANDIAG_CODE_COUNT) {
        return NULL;
    }

    return &diagList[code];
}
const char *DiagGetMsg(PanDiagCode code) { return DiagGetInfo(code)->msg; }

const char *DiagGetHint(PanDiagCode code) {
    const PanDiagInfo *info = DiagGetInfo(code);
    if (info && info->hint) {
        return info->hint;
    } else {
        return NULL;
    }
}
