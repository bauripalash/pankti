#ifndef P_STDLIB_H
#define P_STDLIB_H

#include "env.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct PInterpreter PInterpreter;

void PushStdlib(PInterpreter * it, PEnv * env, const char * name);

#ifdef __cplusplus
}
#endif

#endif
