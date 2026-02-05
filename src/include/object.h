#ifndef OBJECT_H
#define OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ast.h"
#include "ptypes.h"
#include "token.h"

// Forward declaration for PObj
typedef struct PObj PObj;
typedef struct PInterpreter PInterpreter;
typedef struct PEnv PEnv;
typedef struct PBytecode PBytecode;
typedef struct PVm PVm;

// Value Type
typedef enum PValueType { PVAL_NUM, PVAL_OBJ, PVAL_BOOL, PVAL_NIL } PValueType;

#if defined USE_NAN_BOXING
typedef u64 PValue;
#define QNAN       ((u64)0x7ffc000000000000)
#define SIGN_BIT   ((u64)0x8000000000000000)
#define TAG_NIL    1
#define TAG_FALSE  2
#define TAG_TRUE   3
#define NilValue   ((PValue)(u64)(QNAN | TAG_NIL))
#define TrueValue  ((PValue)(u64)(QNAN | TAG_TRUE))
#define FalseValue ((PValue)(u64)(QNAN | TAG_FALSE))
#else
// Stack Allocated PValue
typedef struct PValue {
    PValueType type;
    union {
        double num;
        bool bl;
        PObj *obj;
    } v;
} PValue;
#endif

// typedef PValue (*NativeFn)(PInterpreter *it, PValue *args, u64 argc);
typedef PValue (*NativeFn)(PVm *vm, PValue *args, u64 argc);
// Pankti Object Types
typedef enum PObjType {
    // String Object
    OT_STR,
    // Function Object. Used by `OFunction`
    OT_FNC,
    // Compiler Function Object. Used by `OComFunction`
    OT_COMFNC,
    // Array Object. Used by `OArray`
    OT_ARR,
    // Map Object
    OT_MAP,
    // Native Function Object
    OT_NATIVE,
    // Error Object
    OT_ERROR,
    // Module Object
    OT_MODULE,
    // UpValue Object
    OT_UPVAL,
} PObjType;

// Entry of HashMaps
typedef struct MapEntry {
    u64 key;
    PValue vkey;
    PValue value;
} MapEntry;

// Pankti Object
typedef struct PObj {
    // Pankti Object Type
    PObjType type;
    struct PObj *next;
    bool marked;

    // Union of All Pankti Objects
    union v {
        // String Object. Type : `OT_STR`
        struct OString {
            Token *name;
            char *value;
            // If string is virtual it means the it was created at runtime
            // otherwise the `value` is actually referenced from AST's literal
            bool isVirtual;
            u64 hash;
        } OString;
        // Function Object. Type : `OT_FNC`
        // Gets directly translated from Function Statement : `STMT_FUNC`
        struct OFunction {
            // Name of the Function. (Raw Token)
            Token *name;
            // Token array of parameters
            Token **params;
            // Count of parameters
            u64 paramCount;
            // Function Environment. Will Always have a parent.
            PEnv *env;
            // Array of Statements to execute on demand.
            // Will always be a Block Statement
            PStmt *body;
        } OFunction;

        // Compiled Function Object. Type : `OT_COMFNC`
        struct OComFunction {
            Token *rawName;
            // Will be OString
            PObj *strName;
            u64 paramCount;
            PBytecode *code;
        } OComFunction;

        struct OArray {
            Token *op;
            u64 count;
            PValue *items;
        } OArray;

        struct OMap {
            Token *op;
            u64 count;
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

        // Error Object. Type : `OT_ERROR`
        struct OError {
            // Error message
            char *msg;
        } OError;

        // Object Module. Type : `OT_MODULE`
        struct OModule {
            char *path;
            char *customName;
            u64 nameHash;
        } OModule;

        // UpValue Object. Type : `OT_UPVAL`
        struct OUpval {
            PValue value; // shared value
        } OUpval;
    } v;

} PObj;

// Make a number value
static inline PValue MakeNumber(double number) {
#if defined USE_NAN_BOXING
    PValue val;
    memcpy(&val, &number, sizeof(double));
    return val;
#else
    PValue val;
    val.type = PVAL_NUM;
    val.v.num = number;
    return val;
#endif
}

// Get Numeric value of PValue `value`
static inline double ValueAsNum(PValue value) {
#if defined USE_NAN_BOXING
    double number;
    memcpy(&number, &value, sizeof(PValue));
    return number;
#else
    return value.v.num;
#endif
}

PValueType GetValueType(PValue value);

// Check if value is a number
#if defined(USE_NAN_BOXING)
#define IsValueNum(val) (((val) & QNAN) != QNAN)
#else
#define IsValueNum(val) (val.type == PVAL_NUM)
#endif

// Make a bool value
#if defined(USE_NAN_BOXING)
#define MakeBool(bl) ((bl) ? TrueValue : FalseValue)
#else
static inline PValue MakeBool(bool bl) {
    PValue val;
    val.type = PVAL_BOOL;
    val.v.bl = bl;
    return val;
}
#endif

// Check if value is a bool
#if defined(USE_NAN_BOXING)
#define IsValueBool(val) (((val) | 1) == TrueValue)
#else
#define IsValueBool(val) (val.type == PVAL_BOOL)
#endif

#if defined(USE_NAN_BOXING)
#define ValueAsBool(val) ((val) == TrueValue)
#else
#define ValueAsBool(val) ((bool)val.v.bl)
#endif

// Make a nil value;
#if defined(USE_NAN_BOXING)
#define MakeNil() NilValue
#else
static inline PValue MakeNil(void) {
    PValue val;
    val.type = PVAL_NIL;
    return val;
}
#endif

// Check if value is a nil
#if defined(USE_NAN_BOXING)
#define IsValueNil(val) ((val) == NilValue)
#else
#define IsValueNil(val) (val.type == PVAL_NIL)
#endif

// Make Object value. Wraps the object pointer as Value
#if defined(USE_NAN_BOXING)
#define MakeObject(obj) ((PValue)(SIGN_BIT | QNAN | (u64)(uintptr_t)(obj)))
#else
static inline PValue MakeObject(PObj *obj) {
    PValue val;
    val.type = PVAL_OBJ;
    val.v.obj = obj;
    return val;
}
#endif

// Check if Value is Object. holds the pointer to the heap allocated object
#if defined(USE_NAN_BOXING)
#define IsValueObj(val) (((val) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
#else
#define IsValueObj(val) (val.type == PVAL_OBJ)
#endif

// Typecast value to object. Returns PObj pointer to the heap allocated object
#if defined(USE_NAN_BOXING)
#define ValueAsObj(val) ((PObj *)(uintptr_t)((val) & ~(SIGN_BIT | QNAN)))
#else
#define ValueAsObj(val) ((PObj *)val.v.obj)
#endif

// Check if Value `val` is a object and it is a `otype` object
static inline bool IsValueObjType(PValue val, PObjType otype) {
    if (!IsValueObj(val)) {
        return false;
    }

    PObj *obj = ValueAsObj(val);
    if (obj->type == otype) {
        return true;
    }

    return false;
}

// Return the Message inside the Error Object. Can be NULL
char *GetErrorObjMsg(PObj *obj);
// Check if value is truthy. Only bools are considered
bool IsValueTruthy(PValue val);
// Check if two values `a` and `b` are equal
bool IsValueEqual(PValue a, PValue b);
// Print value to stdout
void PrintValue(PValue val);
// Get string from value
char *ValueToString(PValue val);
// Check if value is a error object
bool IsValueError(PValue val);

// Check if length can be calculated for object
bool ObjectHasLen(PObj *obj);

// Return the length of the object.
// If length can not be calculated for this object return -1
double GetObjectLength(PObj *obj);

u64 GetValueHash(PValue val, u64 seed);
// Can value be used as key for hash map
bool CanValueBeKey(PValue val);
// Can Object be used as key for hash map
bool CanObjectBeKey(PObjType type);
// Get Value Type as String;
// If Value is a object, returns Object Type
const char *ValueTypeToStr(PValue val);

// Check if specified value exists in map
bool MapObjHasKey(PObj *o, PValue key, u64 hash);
// Set or Update key value (with key) pair in map
bool MapObjSetValue(PObj *o, PValue key, u64 keyHash, PValue value);
// Add New Pair or Update existing pair in map
bool MapObjPushPair(PObj *o, PValue key, PValue value, u64 seed);
// Get Value from Map
PValue MapObjGetValue(PObj *map, PValue key, u64 keyHash, bool *found);
bool ArrayObjInsValue(PObj *o, int index, PValue value);
// Push new item to array. Return false if failed to push value.
bool ArrayObjPushValue(PObj *o, PValue value);
// Print Object
void PrintObject(const PObj *o);
// Get string from object
char *ObjToString(PObj *obj);

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
