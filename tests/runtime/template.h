/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTIME_TEST_*_H
#define RUNTIME_TEST_*_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, *){ GoldenTest("*"); }
UTEST(RuntimeTest, *){ GoldenTest("*"); }
UTEST(RuntimeTest, *){ GoldenTest("*"); }



#ifdef __cplusplus
}
#endif

#endif
