/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTIME_TEST_IF_H
#define RUNTIME_TEST_IF_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, IfElse){ GoldenTest("if_else"); }
UTEST(RuntimeTest, IfElseNested){ GoldenTest("if_else_nested"); }



#ifdef __cplusplus
}
#endif

#endif
