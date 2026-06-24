/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "external/stb/stb_ds.h"
#include "gfx.h"
#include "gfx_constants.h"
#include "utils.h"
#include <math.h>

i64 GfxCoreAddImage(PanGfxCore *core, Tigr *img) {
    if (core == NULL) {
        return -1;
    }

    i64 len = (i64)arrlen(core->imageList);
    arrput(core->imageList, img);
    return len;
}

i64 GfxCoreGetImageIndex(const PanGfxCore *core, const char *str, i64 len) {
    if (core == NULL || str == NULL) {
        return -1;
    }

    u64 slen = 0;
    if (len == -1) {
        slen = StrLength(str);
    } else {
        slen = (u64)len;
    }

    if (StrStartsWith(str, GFX_IMAGE_STR_PREFIX)) {
        const char *ptr = str;
        ptr += sizeof(GFX_IMAGE_STR_PREFIX) - 1;
        bool ok = false;
        double rawNum =
            NumberFromStr(ptr, slen - (sizeof(GFX_IMAGE_STR_PREFIX) - 1), &ok);

        if (ok && IsDoubleInt(rawNum) && rawNum >= 0) {
            return (i64)floor(rawNum);
        }
    }

    return -1;
}

Tigr *GfxGetImageFromIdx(const PanGfxCore *core, i64 index, bool *ok) {
    if (core == NULL) {
        *ok = false;
        return NULL;
    }
    if (index < 0 || index >= arrlen(core->imageList)) {
        *ok = false;
        return NULL;
    }

    *ok = true;
    return core->imageList[index];
}

char *GfxGetImageString(const PanGfxCore *core, i64 index, bool *ok) {
    if (core == NULL) {
        *ok = false;
        return NULL;
    }
    if (index < 0 || index >= arrlen(core->imageList)) {
        *ok = false;
        return NULL;
    }

    const char *fmtStr =
        StrFormat("%s%llu", GFX_IMAGE_STR_PREFIX, (unsigned long long)index);
    char *result = StrDuplicate(fmtStr, strlen(fmtStr));

    if (result == NULL) {
        *ok = false;
        return NULL;
    }
    *ok = true;
    return result;
}
