#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include <stdlib.h>

#define PMalloc(size)        (malloc((size_t)size))
#define PCalloc(count, size) (calloc((size_t)count, (size_t)size))
#define PCreate(type)        ((type *)(malloc(sizeof(type))))
#define PFree(ptr)           (free(ptr))

#endif
