/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_FLAGS_H
#define PANKTI_FLAGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
// Debug Flags
typedef struct PanDebugFlags {
    bool lexer;
    bool parser;
    bool bytecode;
    bool times;
    bool gc;
    bool stressGc;
    bool englishNum;
} PanDebugFlags;

extern PanDebugFlags panDebugFlags;

#if defined(PANKTI_BUILD_DEBUG)
void InitDebugFlags(void);
void PrintDebugFlags(void);

#define FLAG_DEBUG_LEXER    (panDebugFlags.lexer)
#define FLAG_DEBUG_PARSER   (panDebugFlags.parser)
#define FLAG_DEBUG_BYTECODE (panDebugFlags.bytecode)
#define FLAG_DEBUG_TIMES    (panDebugFlags.times)
#define FLAG_DEBUG_GC       (panDebugFlags.gc)
#define FLAG_STRESS_GC      (panDebugFlags.stressGc)
#define FLAG_ENGLISH_NUM    (panDebugFlags.englishNum)
#else

#define FLAG_DEBUG_LEXER    0
#define FLAG_DEBUG_PARSER   0
#define FLAG_DEBUG_BYTECODE 0
#define FLAG_DEBUG_TIMES    0
#define FLAG_DEBUG_GC       0
#define FLAG_STRESS_GC      0
#define FLAG_ENGLISH_NUM    0

static inline void InitDebugFlags(void) {}
static inline void PrintDebugFlags(void) {}
#endif
#ifdef __cplusplus
}
#endif

#endif
