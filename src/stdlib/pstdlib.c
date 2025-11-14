#include "../include/pstdlib.h"
#include "../include/interpreter.h"
#include <time.h>

void PushStdlib(PInterpreter * it, PEnv * env, const char * name, StdlibMod mod){
	if (it == NULL || it->gc == NULL) {
		return;
	}

	if (env == NULL) {
		return;
	}
	switch (mod) {
		case STDLIB_OS: PushStdlibOs(it, env);break;
		case STDLIB_MATH: PushStdlibMath(it, env);break;
		case STDLIB_MAP: PushStdlibMap(it, env);break;
		case STDLIB_STRING: PushStdlibString(it, env);break;
		case STDLIB_NONE:break;
	}
}
