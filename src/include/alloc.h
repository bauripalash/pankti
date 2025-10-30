#ifndef ALLOC_H
#define ALLOC_H

#include "stdlib.h"
#include "../external/mimalloc/include/mimalloc.h"


#define PMalloc(size) (mi_malloc(size))
#define PCalloc(count, size) (mi_calloc(count, size))
#define PCreate(type) ((type *)(mi_malloc(sizeof(type))))
#define PFree(ptr)    (mi_free(ptr))

/*
#define PMalloc(size) (malloc(size))
#define PCalloc(count, size) (calloc(count, size))
#define PCreate(type) ((type *)(malloc(sizeof(type))))
#define PFree(ptr)    (free(ptr))
*/
#endif
