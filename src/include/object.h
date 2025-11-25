#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"
#include "token.h"
#include <stdbool.h>

// Forward declration for PObj
typedef struct PObj PObj;
typedef struct PInterpreter PInterpreter;

// Value Type
typedef enum PValueType { VT_NUM, VT_OBJ, VT_BOOL, VT_NIL } PValueType;

// Stack Allocated PValue
typedef struct PValue {
    PValueType type;
    union {
        double num;
        bool bl;
        PObj *obj;
    } v;
} PValue;

typedef PValue (*NativeFn)(PInterpreter *it, PValue *args, size_t argc);

// Pankti Object Types
typedef enum PObjType {
    // String Object
    OT_STR,
    // Function Object. Used by `OFunction`
    OT_FNC,
    // Array Object. Used by `OArray`
    OT_ARR,
    OT_MAP,
    OT_NATIVE,
} PObjType;

// Entry of HashMaps
typedef struct MapEntry {
    uint64_t key;
    PValue vkey;
    PValue value;
} MapEntry;

// Pankti Object
typedef struct PObj {
    // Pankti Object Type
    PObjType type;
    struct PObj *next;

    // Union of All Pankti Objects
    union v {
        // String Object. Type : `OT_STR`
        struct OString {
            Token *name;
            char *value;
            bool isVirtual;
        } OString;
        // Function Object. Type : `OT_FNC`
        // Gets directly translated from Function Statement : `STMT_FUNC`
        struct OFunction {
            // Name of the Function. (Raw Token)
            Token *name;
            // Token array of parameters
            Token **params;
            // Count of parameters
            size_t paramCount;
            // Function Environment. Will Always have a parent.
            void *env;
            // Array of Statements to execute on demand.
            // Will always be a Block Statement
            PStmt *body;
        } OFunction;

        struct OArray {
            Token *op;
            size_t count;
            PValue *items;
        } OArray;

        struct OMap {
            Token *op;
            size_t count;
            MapEntry *table;
        } OMap;

        // Native Function Object. Type : `OT_NATIVE`
        struct ONative {
            // Native `C` Function
            NativeFn fn;
            // How many args the function needs
            int arity;
            // Function name (Raw Token)
            Token *name;
        } ONative;
    } v;

} PObj;

// Check if value is a number
#define IsValueNum(val) (val.type == VT_NUM)
// Check if value is a bool
#define IsValueBool(val) (val.type == VT_BOOL)
// Check if value is a nil
#define IsValueNil(val) (val.type == VT_NIL)
// Check if Value is Object. holds the pointer to the heap allocated object
#define IsValueObj(val) (val.type == VT_OBJ)

// Typecast value to number (double)
#define ValueAsNum(val) ((double)val.v.num)
// Typecast value to bool value
#define ValueAsBool(val) ((bool)val.v.bl)
// Typecast value to object. Returns PObj pointer to the heap allocated object
#define ValueAsObj(val) ((PObj *)val.v.obj)

// Make a number value
static inline PValue MakeNumber(double value) {
    PValue val;
    val.type = VT_NUM;
    val.v.num = value;
    return val;
}

// Make a bool value
static inline PValue MakeBool(bool bl) {
    PValue val;
    val.type = VT_BOOL;
    val.v.bl = bl;
    return val;
}
// Make a nil value
static inline PValue MakeNil(void) {
    PValue val;
    val.type = VT_NIL;
    return val;
}

// Make Object value. Wraps the object pointer as Value
static inline PValue MakeObject(PObj *obj) {
    PValue val;
    val.type = VT_OBJ;
    val.v.obj = obj;
    return val;
}

// Check if Value `val` is a object and it is a `otype` object
static inline bool IsValueObjType(const PValue *val, PObjType otype) {
    if (val->type != VT_OBJ) {
        return false;
    }

    if (val->v.obj->type != otype) {
        return false;
    }

    return true;
}
// Check if value is truthy. Only bools are considered
bool IsValueTruthy(const PValue *val);
// Check if two values `a` and `b` are equal
bool IsValueEqual(const PValue *a, const PValue *b);
// Print value to stdout
void PrintValue(const PValue *val);

uint64_t GetValueHash(const PValue *val, uint64_t seed);
// Can value be used as key for hash map
bool CanValueBeKey(const PValue *val);
// Can Object be used as key for hash map
bool CanObjectBeKey(PObjType type);
// Get Value Type as String;
// If Value is a object, returns Object Type
const char *ValueTypeToStr(const PValue *val);

bool MapObjSetValue(PObj *o, PValue key, uint64_t keyHash, PValue value);
bool ArrayObjInsValue(PObj *o, int index, PValue value);
// Print Object
void PrintObject(const PObj *o);

// Get Object type as String
char *ObjTypeToString(PObjType type);

// Check if two objects `a` and `b` are equal
// `a` = first object
// `b` = second object
bool IsObjEqual(const PObj *a, const PObj *b);

#ifdef __cplusplus
}
#endif

#endif
