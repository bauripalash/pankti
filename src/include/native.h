/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NATIVE_H
#define NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct PVm PVm;
void RegisterNatives(PVm *vm);

#ifdef __cplusplus
}
#endif

#endif
