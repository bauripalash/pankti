#ifndef RUNTIME_TEST_INTERNAL_H
#define RUNTIME_TEST_INTERNAL_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, Internal_Types){ GoldenTest("types"); }
UTEST(RuntimeTest, Internal_ScriptWithBOM){ GoldenTest("arithmetic_bom"); }


#ifdef __cplusplus
}
#endif

#endif
