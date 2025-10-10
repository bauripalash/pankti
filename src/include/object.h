#ifndef OBJECT_H
#define OBJECT_H
#include <stdbool.h>

typedef enum PObjType{
	OT_NUM,
	OT_STR,
	OT_BOOL,
	OT_NIL
}PObjType;

typedef struct PObj{
	PObjType type;

	union v{
		double num;
		char * str;
		bool bl;
	}v;

}PObj;

PObj * NewObject(PObjType type);
void FreeObject(PObj * o);
void PrintObject(const PObj * o);


PObj * NewNumberObj(double value);
PObj * NewBoolObj(bool value);
PObj * NewStrObject(char * value);


bool IsObjTruthy(const PObj * o);
bool isObjEqual(const PObj * a, const PObj * b);

#endif
