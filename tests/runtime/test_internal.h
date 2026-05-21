#ifndef RUNTIME_TEST_INTERNAL_H
#define RUNTIME_TEST_INTERNAL_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, InternalTypes){ GoldenTest("types"); }


#ifdef __cplusplus
}
#endif

#endif
