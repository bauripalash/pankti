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
	OT_RET,
	OT_BRK,
}PObjType;



typedef struct PObj{
	PObjType type;

	union v{
		double num;
		char * str;
		bool bl;

		struct OReturn{
			struct PObj * rvalue;
		}OReturn;

		struct OBreak{}OBreak;
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
PObj * NewReturnObject(PObj * value);
PObj * NewBreakObject();


bool IsObjTruthy(const PObj * o);
bool isObjEqual(const PObj * a, const PObj * b);

#endif
