#ifndef NATIVE_H
#define NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "env.h"
#include "object.h"

void RegisterNatives(PInterpreter *it, PEnv *env);

#ifdef __cplusplus
}
#endif

#endif
