#ifndef ENV_H
#define ENV_H

#include "object.h"
#include "token.h"
#include <stdint.h>
typedef struct PEnv{
	struct table{ uint32_t key; PObj * value; } * table;
	int count;
	struct PEnv * enclosing;
}PEnv;

#define GetEnv(ptr) ((PEnv*)ptr)
PEnv * NewEnv(PEnv * enclosing);
void FreeEnv(PEnv * e);

//Set/Create New Variable-Value pair
void EnvPutValue(PEnv * e, uint32_t hash, PObj*value);

//Get Value;
PObj * EnvGetValue(PEnv * e, uint32_t hash);

//Set value to existing variable; return false on fail
bool EnvSetValue(PEnv * e, uint32_t hash, PObj * value);

#endif
