#ifndef ENV_H
#define ENV_H

#include "object.h"
#include "token.h"
#include <stdint.h>
typedef struct PEnv{
	struct table{ uint32_t key; PObj * value; } * table;
	int count;
}PEnv;

PEnv * NewEnv();
void FreeEnv(PEnv * e);
void EnvPutValue(PEnv * e, uint32_t hash, PObj*value);
PObj * EnvGetValue(PEnv * e, uint32_t hash);

#endif
