#ifndef ENV_H
#define ENV_H

#include "object.h"
#include <stdint.h>
typedef struct EnvPair{
	uint32_t key;
	PObj * value;
}EnvPair;
typedef struct PEnv{
	EnvPair ** table;
	int count;
	struct PEnv * enclosing;
}PEnv;

#define GetEnv(ptr) ((PEnv*)ptr)
PEnv * NewEnv(PEnv * enclosing);
EnvPair * NewPair(uint32_t key, PObj * value);
void FreeEnv(PEnv * e);
void DebugEnv(PEnv * e);

//Set/Create New Variable-Value pair
void EnvPutValue(PEnv * e, uint32_t hash, PObj*value);

//Get Value;
PObj * EnvGetValue(PEnv * e, uint32_t hash);

//Set value to existing variable; return false on fail
bool EnvSetValue(PEnv * e, uint32_t hash, PObj * value);

#endif
