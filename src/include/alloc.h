#ifndef ALLOC_H
#define ALLOC_H

#include "stdlib.h"

#define PMalloc(size)        (malloc(size))
#define PCalloc(count, size) (calloc(count, size))
#define PCreate(type)        ((type *)(malloc(sizeof(type))))
#define PFree(ptr)           (free(ptr))

#endif
