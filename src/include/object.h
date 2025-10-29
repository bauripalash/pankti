#ifndef OBJECT_H
#define OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"
#include "token.h"
#include <stdbool.h>

// Pankti Object Types
typedef enum PObjType {
    // Number Object
    OT_NUM,
    // String Object
    OT_STR,
    // Bool Object
    OT_BOOL,
    // Nil Object
    OT_NIL,
    // Return Object. Used by `OReturn`
    OT_RET,
    // Break Object. Used by `OBreak`
    OT_BRK,
    // Function Object. Used by `OFunction`
    OT_FNC,
} PObjType;

// Pankti Object
typedef struct PObj {
    // Pankti Object Type
    PObjType type;
    struct PObj *next;

    // Union of All Pankti Objects
    union v {
        // Number Object. Type : `OT_NUM`
        double num;
        // String Object. Type : `OT_STR`
        char *str;
        // Bool Object. Type : `OT_BOOL`
        bool bl;
        // Return Object. Type : `OT_RET`
        struct OReturn {
            // The Actual Value
            struct PObj *rvalue;
        } OReturn;

        // Break Object. Type : `OT_BRK`
        struct OBreak {

        } OBreak;

        // Function Object. Type : `OT_FNC`
        // Gets directly translated from Function Statement : `STMT_FUNC`
        struct OFunction {
            // Name of the Function. (Raw Token)
            Token *name;
            // Token array of parameters
            Token **params;
            // Count of parameters
            int paramCount;
            // Function Environment. Will Always have a parent.
            void *env;
            // Array of Statements to execute on demand.
            // Will always be a Block Statement
            PStmt *body;
        } OFunction;
    } v;

} PObj;

typedef enum PValueType{
	VT_NUM,
	VT_OBJ,
	VT_BOOL,
	VT_NIL
}PValueType;

typedef struct PValue{
	PValueType type;
	union{
		double num;
		bool bl;
		PObj * obj;
	}v;
}PValue;


#define IsValueNum(val) (val.type == VT_NUM)
#define IsValueBool(val) (val.type == VT_BOOL)
#define IsValueNum(val) (val.type == VT_NUM)
#define IsValueObj(val) (val.type == VT_OBJ)

#define ValueAsNum(val) ((double)val.v.num)
#define ValueAsBool(val) ((bool)val.v.bl)
#define ValueAsObj(val) ((PObj*)val.v.obj)

static inline PValue MakeNumber(double value){
	PValue val; val.type = VT_NUM; val.v.num = value; return val;
}
static inline PValue MakeBool(bool bl){
	PValue val; val.type = VT_BOOL; val.v.bl = bl; return val;
}
static inline PValue MakeNil(){
	PValue val; val.type = VT_NIL; return val;
}
static inline PValue MakeObject(PObj * obj){
	PValue val; val.type = VT_OBJ; val.v.obj = obj; return val;
}

bool IsValueEqual(const PValue * a, const PValue * b);
void PrintValue(const PValue * val);

// Print Object
void PrintObject(const PObj *o);

// Get Object type as String
char *ObjTypeToString(PObjType type);
// Check if the object is Truthy
// `obj` = Object to check
// Will only return True if the object is Truthy.
// In Pankti, only Bool object with value True will result to True return
// Otherwise the return will be False
bool IsObjTruthy(const PObj *obj);

// Check if two objects `a` and `b` are equal
// `a` = first object
// `b` = second object
bool IsObjEqual(const PObj *a, const PObj *b);

#ifdef __cplusplus
}
#endif

#endif
