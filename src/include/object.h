#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
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


#if defined USE_NAN_BOXING
typedef uint64_t PValue;
#define QNAN ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define TAG_NIL   1
#define TAG_FALSE 2
#define TAG_TRUE  3
#define NilValue ((PValue)(uint64_t)(QNAN | TAG_NIL))
#define TrueValue ((PValue)(uint64_t)(QNAN | TAG_TRUE))
#define FalseValue ((PValue)(uint64_t)(QNAN | TAG_FALSE))
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
	OT_ERROR,
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

		// Error Object. Type : `OT_ERROR`
		struct OError {
			// Error message
			char * msg;
		} OError;
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
	val.type = VT_NUM;
	val.v.num = number;
	return val;
#endif
}

// Get Numeric value of PValue `value`
static inline double ValueAsNum(PValue value){
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
#if defined (USE_NAN_BOXING)
#define IsValueNum(val) (((val) & QNAN) != QNAN)
#else 
#define IsValueNum(val) (val.type == VT_NUM)
#endif

// Make a bool value
#if defined (USE_NAN_BOXING)
#define MakeBool(bl) ((bl) ? TrueValue : FalseValue)
#else
static inline PValue MakeBool(bool bl) {
    PValue val;
    val.type = VT_BOOL;
    val.v.bl = bl;
    return val;
}
#endif

// Check if value is a bool
#if defined (USE_NAN_BOXING)
#define IsValueBool(val) (((val) | 1) == TrueValue)
#else
#define IsValueBool(val) (val.type == VT_BOOL)
#endif

#if defined (USE_NAN_BOXING)
#define ValueAsBool(val) ((val) == TrueValue)
#else
#define ValueAsBool(val) ((bool)val.v.bl)
#endif

// Make a nil value;
#if defined (USE_NAN_BOXING)
#define MakeNil() NilValue
#else
static inline PValue MakeNil(void) {
    PValue val;
    val.type = VT_NIL;
    return val;
}
#endif

// Check if value is a nil
#if defined (USE_NAN_BOXING)
#define IsValueNil(val) ((val) == NilValue)
#else
#define IsValueNil(val) (val.type == VT_NIL)
#endif

// Make Object value. Wraps the object pointer as Value
#if defined (USE_NAN_BOXING)
#define MakeObject(obj) ((PValue)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj)))
#else
static inline PValue MakeObject(PObj *obj) {
    PValue val;
    val.type = VT_OBJ;
    val.v.obj = obj;
    return val;
}
#endif

// Check if Value is Object. holds the pointer to the heap allocated object
#if defined (USE_NAN_BOXING)
#define IsValueObj(val) (((val) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
#else
#define IsValueObj(val) (val.type == VT_OBJ)
#endif

// Typecast value to object. Returns PObj pointer to the heap allocated object
#if defined (USE_NAN_BOXING)
#define ValueAsObj(val) ((PObj*)(uintptr_t)((val) & ~(SIGN_BIT | QNAN)))
#else
#define ValueAsObj(val) ((PObj *)val.v.obj)
#endif

// Check if Value `val` is a object and it is a `otype` object
static inline bool IsValueObjType(PValue val, PObjType otype) {
	if (!IsValueObj(val)) {
		return false;
	}

	PObj * obj = ValueAsObj(val);
	if (obj->type == otype) {
		return true;
	}

    return false;
}


// Return the Message inside the Error Object. Can be NULL
char * GetErrorObjMsg(PObj * obj);
// Check if value is truthy. Only bools are considered
bool IsValueTruthy(PValue val);
// Check if two values `a` and `b` are equal
bool IsValueEqual(PValue a, PValue b);
// Print value to stdout
void PrintValue(PValue val);
// Check if value is a error object
bool IsValueError(PValue val);


// Check if length can be calculated for object
bool ObjectHasLen(PObj * obj);

// Return the length of the object.
// If length can not be calculated for this object return -1
double GetObjectLength(PObj * obj);

uint64_t GetValueHash(PValue val, uint64_t seed);
// Can value be used as key for hash map
bool CanValueBeKey(PValue val);
// Can Object be used as key for hash map
bool CanObjectBeKey(PObjType type);
// Get Value Type as String;
// If Value is a object, returns Object Type
const char *ValueTypeToStr(PValue val);

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
