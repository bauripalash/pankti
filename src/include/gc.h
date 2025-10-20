#ifndef GC_H
#define GC_H

#include "object.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#define DEBUG_GC true

//Pankti Garbage Collector
typedef struct Pgc{
	bool disable;
	bool stress;
	PObj * objects;
	size_t nextGc;

	PObj * onlyNilObj;
	PObj * onlyTrueObj;
	PObj * onlyFalseObj;
}Pgc;


Pgc * NewGc();
void FreeGc(Pgc * gc);

// Create a New Empty Object.
// `type` = Type of statement
PObj *NewObject(Pgc * gc, PObjType type);

// Free the Object
// Handle all underlying values according to function type.
void FreeObject(Pgc * gc, PObj *o);


// Create New Number Object
// `value` = Numerical floating point value of the object
PObj *NewNumberObj(Pgc * gc, double value);

// Create New Bool Object
// `value` = Bool value
PObj *NewBoolObj(Pgc * gc, bool value);

// Create New String Object
// `value` = String value
PObj *NewStrObject(Pgc * gc, char *value);

// New Nil Object
PObj *NewNilObject(Pgc * gc);

// New Return Statement Object
// `value` = The actual value of the return statement.
// Value will be Nil Object if it is empty return statement
// Only used in functions
PObj *NewReturnObject(Pgc * gc, PObj *value);

// New Break Statement Object
// Only used in while loops
PObj *NewBreakObject(Pgc * gc);

// Create Function Statement Object
// `name` = Name of the function
// `params` = Token array of parameters
// `body` = Body of function. Statement will always be Block Statement
// `env` = Environment
// `count` = Count of parameters
PObj *
NewFuncObject(Pgc * gc, Token *name, Token **params, PStmt *body, void *env, int count);




#ifdef __cplusplus
}
#endif

#endif
