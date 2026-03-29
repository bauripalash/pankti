#ifndef PANKTI_FRONTEND_H
#define PANKTI_FRONTEND_H

#include "gc.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct PFrontend {
    Pgc *gc;
} PFrontend;

#ifdef __cplusplus
}
#endif

#endif
