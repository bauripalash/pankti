/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../external/stb/stb_ds.h"
#include "../include/object.h"
#include <stdbool.h>
#include <stdlib.h>

bool ArrayObjInsValue(PObj *o, int index, PValue value) {
    if (o == NULL) {
        return false;
    }

    if (o->type != OT_ARR) {
        return false;
    }

    struct OArray *arr = &o->v.OArray;
    if (index < 0 || index >= arr->count) {
        return false;
    }

    arrput(arr->items, value);
    arrdelswap(arr->items, index);
    arr->count = (u64)arrlen(arr->items);

    return true;
}

bool ArrayObjPushValue(PObj *o, PValue value) {
    if (o == NULL || o->type != OT_ARR) {
        return false;
    }

    struct OArray *arr = &o->v.OArray;
    u64 oldCount = arr->count;
    arrput(arr->items, value);
    u64 newCount = arrlen(arr->items);

    if (newCount == oldCount + 1) {
        arr->count = newCount;
        return true;
    } else {
        return false;
    }
}
