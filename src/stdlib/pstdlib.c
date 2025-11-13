#include "../include/pstdlib.h"
#include "../include/interpreter.h"
#include "../include/utils.h"
#include <math.h>
#include <time.h>

static PValue math_Pow (PInterpreter * it, PValue *args, int argc){
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


void PushStdlib(PInterpreter * it, PEnv * env, const char * name){
	if (it == NULL || it->gc == NULL) {
		return;
	}

	if (env == NULL) {
		return;
	}
	//Name check
	
	PObj * powFn = NewNativeFnObject(it->gc, NULL, math_Pow, 2);
	EnvPutValue(env, StrHash("pow",3, it->gc->timestamp), MakeObject(powFn));
}
