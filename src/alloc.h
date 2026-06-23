/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include <stdlib.h>

#define PMalloc(size)        (malloc((size_t)size))
#define PCalloc(count, size) (calloc((size_t)count, (size_t)size))
#define PRealloc(ptr, size)  (realloc(ptr, (size_t)(size)))
#define PCreate(type)        ((type *)(malloc(sizeof(type))))
#define PCreateArray(type, count)                                              \
    ((type *)(malloc(sizeof(type) * (size_t)(count))))
#define PFree(ptr) (free(ptr))

#endif
