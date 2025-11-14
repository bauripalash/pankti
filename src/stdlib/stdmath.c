#include "../include/pstdlib.h"
#include "../include/env.h"
#include "../include/interpreter.h"
#include <math.h>

static PValue math_Pow(PInterpreter * it, PValue *args, int argc){
	PValue * a = &args[0];
	PValue * b = &args[1];

	if (a->type == VT_NUM && b->type == VT_NUM) {
		double av = a->v.num;
		double bv = b->v.num;
		double result = pow(av, bv);
		return MakeNumber(result);
	}

	return MakeNil();
}

static const StdlibEntry entries[] = {
	(StdlibEntry){.name = "pow", .nlen = 3, .fn = math_Pow, .arity = 2}
};

void PushStdlibMath(PInterpreter *it, PEnv * env){
	int count = ArrCount(entries);
	for (int i = 0; i < count; i++) {
		const StdlibEntry * entry = &entries[i];
		PObj * stdFn = NewNativeFnObject(it->gc, NULL, entry->fn, entry->arity);
		EnvPutValue(env, StrHash(entry->name, entry->nlen, it->gc->timestamp), MakeObject(stdFn));
	}
}
