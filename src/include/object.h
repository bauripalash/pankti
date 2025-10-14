#ifndef OBJECT_H
#define OBJECT_H
#include "ast.h"
#include "token.h"
#include <stdbool.h>

typedef enum PObjType{
	//Number
	OT_NUM,
	//String
	OT_STR,
	//Bool
	OT_BOOL,
	//Nil
	OT_NIL,
	//Return
	OT_RET,
	//Break
	OT_BRK,
	//Function
	OT_FNC,
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

		struct OFunction{
			Token * name;
			Token ** params;
			int paramCount;
			void * env;
			PStmt * body;
		}OFunction;
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
PObj * NewFuncObject(Token * name, Token ** params, PStmt * body, void * env, int count);


bool IsObjTruthy(const PObj * o);
bool isObjEqual(const PObj * a, const PObj * b);

#endif
