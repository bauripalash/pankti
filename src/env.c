#include "include/env.h"
#include "include/alloc.h"
#include "external/stb/stb_ds.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

PEnv * NewEnv(PEnv * enclosing){
	PEnv * e = PCreate(PEnv);
	//error check;
	e->count = 0;
	e->table = NULL;
	e->enclosing = enclosing;
	hmdefault(e->table, NULL);
	return e;
}
void FreeEnv(PEnv * e){
	if (e == NULL) {
		return;
	}
	hmfree(e->table);
	e->table = NULL;

	free(e);
}
void EnvPutValue(PEnv * e, uint32_t hash, PObj*value){
	hmput(e->table, hash, value);
	e->count++;
}

bool EnvSetValue(PEnv * e, uint32_t hash, PObj * value){
	if (hmget(e->table, hash) != NULL) {
		hmput(e->table, hash, value);
		return true;
	}

	if (e->enclosing != NULL) {
		return EnvSetValue(e->enclosing, hash, value);
	}

	return false;
}

PObj * EnvGetValue(PEnv * e, uint32_t hash){
	PObj * value = hmget(e->table, hash);
	if (value != NULL) {
		return value;
	}

	if (e->enclosing != NULL) {
		return EnvGetValue(e->enclosing, hash);
	}

	return NULL;
}
