#ifndef RUNTIME_TEST_IMPORTS_H
#define RUNTIME_TEST_IMPORTS_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, ImportAlias){ GoldenTest("import_alias"); }
UTEST(RuntimeTest, ImportMultiple){ GoldenTest("import_multiple"); }



#ifdef __cplusplus
}
#endif

#endif
