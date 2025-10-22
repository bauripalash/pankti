#ifndef ALLOC_H
#define ALLOC_H

#include "stdlib.h"

#define PCreate(type) (type *)(malloc(sizeof(type)))
#define PFree(ptr)    (free(ptr))

#endif
