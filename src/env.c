#include "include/env.h"
#include "include/alloc.h"
#include "external/stb/stb_ds.h"
#include <stdlib.h>

PEnv * NewEnv(){
	PEnv * e = PCreate(PEnv);
	//error check;
	e->count = 0;
	e->table = NULL;
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
PObj * EnvGetValue(PEnv * e, uint32_t hash){
	return hmget(e->table, hash);
}
