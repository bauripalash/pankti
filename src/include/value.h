#ifndef VALUE_H
#define VALUE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "object.h"

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

void PrintValue(const PValue * val);



#ifdef __cplusplus
}
#endif
#endif
