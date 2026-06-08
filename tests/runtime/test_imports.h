/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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
