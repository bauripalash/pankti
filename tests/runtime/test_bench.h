/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTIME_TEST_BENCH_H
#define RUNTIME_TEST_BENCH_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, Benchmarks_Array){ GoldenTest("bench_array"); }
UTEST(RuntimeTest, Benchmarks_Fib){ GoldenTest("bench_fib"); }
UTEST(RuntimeTest, Benchmarks_Loop){ GoldenTest("bench_loop"); }
UTEST(RuntimeTest, Benchmarks_NestCall){ GoldenTest("bench_nestcall"); }
UTEST(RuntimeTest, Benchmarks_String){ GoldenTest("bench_string"); }



#ifdef __cplusplus
}
#endif

#endif
