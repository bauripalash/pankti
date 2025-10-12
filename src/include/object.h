#ifndef OBJECT_H
#define OBJECT_H
#include "ast.h"
#include "token.h"
#include <stdbool.h>

typedef enum PObjType{
	OT_NUM,
	OT_STR,
	OT_BOOL,
	OT_NIL,
	OT_FUNC,
	OT_META
}PObjType;



typedef struct PObj{
	PObjType type;

	union v{
		double num;
		char * str;
		bool bl;
		struct OFunc{
			//PEnv* env
			void * env;
			PStmt * body;
			Token ** params;
			Token * name;
		}OFunc;

		struct _OMeta{
			struct PObj * ret;
		}_OMeta;
	}v;

}PObj;

PObj * NewObject(PObjType type);
void FreeObject(PObj * o);
void PrintObject(const PObj * o);
char * ObjTypeToString(PObjType type);


PObj * NewNumberObj(double value);
PObj * NewBoolObj(bool value);
PObj * NewStrObject(char * value);
PObj * NewNilObject();
PObj * NewFuncObject(Token * name, Token ** params, PStmt * body, void * env);
PObj * NewOMeta();


bool IsObjTruthy(const PObj * o);
bool isObjEqual(const PObj * a, const PObj * b);

#endif
