/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_ARGPARSE_H
#define PANKTI_ARGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PanArgsResult {
    PARGS_OK = 0,
    PARGS_EXIT_OK = 1,
    PARGS_EXIT_ERR = 2
} PanArgsResult;

typedef struct PanktiArgs {
    const char *scriptPath;
    const char *evalCode;
    char **scriptArgs;
    int scriptArgCount;
} PanktiArgs;

PanArgsResult ParsePanArgs(int argc, char **argv, PanktiArgs *out);

void PrintPanktiVersion(void);
void PrintPanktiHelp(void);

#ifdef __cplusplus
}
#endif

#endif
