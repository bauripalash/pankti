#include "include/object.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
    case OT_NUM:
        printf("%f", o->v.num);
        break;
    case OT_STR:
        printf("%s", o->v.str);
        break;
    case OT_BOOL:
        printf("%s", o->v.bl ? "true" : "false");
        break;
    case OT_NIL:
        printf("nil");
        break;
    case OT_RET: {
        printf("<ret ");
        PrintObject(o->v.OReturn.rvalue);
        printf(">");
        break;
    }
    case OT_BRK: {
        printf("<break>\n");
        break;
    }
    case OT_FNC: {
        printf("<fn %s>", o->v.OFunction.name->lexeme);
        break;
    }
    }
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
    case OT_NUM:
        return "Number";
        break;
    case OT_STR:
        return "String";
        break;
    case OT_BOOL:
        return "Bool";
        break;
    case OT_NIL:
        return "Nil";
        break;
    case OT_RET:
        return "Return";
        break;
    case OT_BRK:
        return "Break";
        break;
    case OT_FNC:
        return "Function";
        break;
    }

    return "";
}



bool IsObjTruthy(const PObj *o) {
    if (o != NULL && o->type == OT_BOOL) {
        return o->v.bl;
    }

    return false;
}

bool isObjEqual(const PObj *a, const PObj *b) {
    if (a->type != b->type) {
        return false;
    }
    bool result = false;
    switch (a->type) {
    case OT_NUM: {
        result = (a->v.num == b->v.num);
        break;
    }
    case OT_BOOL: {
        result = (a->v.bl == b->v.bl);
        break;
    }

    case OT_NIL: {
        result = true;
        break;
    }

    case OT_STR: {
        result = StrEqual(a->v.str, b->v.str);
        break;
    }
    case OT_RET: {
        result = isObjEqual(a->v.OReturn.rvalue, a->v.OReturn.rvalue);
        break;
    }

    case OT_BRK: {
        result = true;
        break;
    }

    case OT_FNC:
        result = false;
        break;
    }

    return result;
}

