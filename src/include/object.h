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

// Create a New Empty Object.
// `type` = Type of statement
PObj *NewObject(PObjType type);

// Free the Object
// Handle all underlying values according to function type.
void FreeObject(PObj *o);

// Print Object
void PrintObject(const PObj *o);

// Get Object type as String
char *ObjTypeToString(PObjType type);

// Create New Number Object
// `value` = Numerical floating point value of the object
PObj *NewNumberObj(double value);

// Create New Bool Object
// `value` = Bool value
PObj *NewBoolObj(bool value);

// Create New String Object
// `value` = String value
PObj *NewStrObject(char *value);

// New Nil Object
PObj *NewNilObject();

// New Return Statement Object
// `value` = The actual value of the return statement.
// Value will be Nil Object if it is empty return statement
// Only used in functions
PObj *NewReturnObject(PObj *value);

// New Break Statement Object
// Only used in while loops
PObj *NewBreakObject();

// Create Function Statement Object
// `name` = Name of the function
// `params` = Token array of parameters
// `body` = Body of function. Statement will always be Block Statement
// `env` = Environment
// `count` = Count of parameters
PObj *
NewFuncObject(Token *name, Token **params, PStmt *body, void *env, int count);

// Check if the object is Truthy
// `obj` = Object to check
// Will only return True if the object is Truthy.
// In Pankti, only Bool object with value True will result to True return
// Otherwise the return will be False
bool IsObjTruthy(const PObj *obj);

// Check if two objects `a` and `b` are equal
// `a` = first object
// `b` = second object
bool isObjEqual(const PObj *a, const PObj *b);

#ifdef __cplusplus
}
#endif

#endif
