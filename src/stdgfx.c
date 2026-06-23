/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "external/stb/stb_ds.h"
#include "external/tigr/tigr.h"
#include "gc.h"
#include "gfxcore.h"
#include "gfxdraw.h"
#include "gfxfont.h"
#include "gfxhelper.h"
#include "printer.h"
#include "pstdlib.h"
#include "symtable.h"
#include "vm.h"
#include <stdbool.h>

static PanGfxCore *gcore = NULL;
float deltaTime = 0.0;

static PValue gfx_New(PVm *vm, PValue *args, u64 argc) {
    PValue rawWinH = args[0];
    if (!IsValueNum(rawWinH)) {
        VmError(vm, RT_STDGFX_NEW_HEIGHT_INVALID_TYPE, ValueTypeToStr(rawWinH));
        return MakeNil();
    }

    double winH = ValueAsNum(rawWinH);

    if (!IsDoubleSafeInt(winH)) {
        VmError(vm, RT_STDGFX_NEW_HEIGHT_INVALID_INT_RANGE, winH);
        return MakeNil();
    }

    if (winH <= 0 || !IsDoubleInt(winH)) {
        VmError(vm, RT_STDGFX_NEW_HEIGHT_NOT_POS_INT);
        return MakeNil();
    }

    PValue rawWinW = args[1];

    if (!IsValueNum(rawWinW)) {
        VmError(vm, RT_STDGFX_NEW_WIDTH_INVALID_TYPE, ValueTypeToStr(rawWinW));
        return MakeNil();
    }

    double winW = ValueAsNum(rawWinW);

    if (!IsDoubleSafeInt(winW)) {
        VmError(vm, RT_STDGFX_NEW_WIDTH_INVALID_INT_RANGE, winW);
        return MakeNil();
    }

    if (winW <= 0 || !IsDoubleInt(winW)) {
        VmError(vm, RT_STDGFX_NEW_WIDTH_NOT_POS_INT);
        return MakeNil();
    }

    PValue rawTitle = args[2];
    if (!IsValueObjType(rawTitle, OT_STR)) {
        VmError(vm, RT_STDGFX_NEW_TITLE_INVALID_TYPE, ValueTypeToStr(rawTitle));
        return MakeNil();
    }

    char *title = ValueAsObj(rawTitle)->v.OString.value;

    if (gcore == NULL) {
        gcore = NewGfxCore(winW, winH, title, -1);
    }

    StartGfxProcess(gcore);
    return MakeNil();
}

static PValue gfx_Stop(PVm *vm, PValue *args, u64 argc) {
    EndGfxProcess(gcore);
    FreeGfxCore(gcore);
    return MakeNil();
}

static PValue gfx_Running(PVm *vm, PValue *args, u64 argc) {
    UpdateGfxStatus(gcore);
    return MakeBool(gcore->winRunning);
}

static PValue gfx_DrawStart(PVm *vm, PValue *args, u64 argc) {

    deltaTime = tigrTime();
    return MakeNil();
}

static PValue gfx_DrawFinish(PVm *vm, PValue *args, u64 argc) {
    tigrUpdate(gcore->screen);
    PanFlushStdout();
    PanFlushStderr();
    return MakeNil();
}

static PValue gfx_DrawLine(PVm *vm, PValue *args, u64 argc) {
    PValue rawX1 = args[0];
    PValue rawY1 = args[1];
    PValue rawX2 = args[2];
    PValue rawY2 = args[3];
    PValue rawColor = args[4];

    if (!IsValueNum(rawX1)) {
        VmError(vm, RT_STDGFX_LINE_X1_INVALID_TYPE, ValueTypeToStr(rawX1));
        return MakeNil();
    }

    double x1Val = ValueAsNum(rawX1);
    double y1Val = ValueAsNum(rawY1);
    double x2Val = ValueAsNum(rawX2);
    double y2Val = ValueAsNum(rawY2);

    char *colorStr = ValueAsObj(rawColor)->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_LINE_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }

    GfxDrawLine(gcore, (int)x1Val, (int)y1Val, (int)x2Val, (int)y2Val, 1, clr);

    return MakeNil();
}

static PValue gfx_DrawPixel(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawColor = args[2];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_PIXEL_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_PIXEL_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }

    if (!IsValueObjType(rawColor, OT_STR)) {
        VmError(
            vm, RT_STDGFX_PIXEL_COLOR_INVALID_TYPE, ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;

    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_PIXEL_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }
    GfxDrawPixel(gcore, (int)xVal, (int)yVal, clr);
    return MakeNil();
}

static PValue gfx_DrawRectangle(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawH = args[2];
    PValue rawW = args[3];
    PValue rawColor = args[4];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_DRAWRECT_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }
    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_DRAWRECT_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }
    if (!IsValueNum(rawH)) {
        VmError(vm, RT_STDGFX_DRAWRECT_H_INVALID_TYPE, ValueTypeToStr(rawH));
        return MakeNil();
    }
    if (!IsValueNum(rawW)) {
        VmError(vm, RT_STDGFX_DRAWRECT_W_INVALID_TYPE, ValueTypeToStr(rawW));
        return MakeNil();
    }
    if (!IsValueObjType(rawColor, OT_STR)) {
        VmError(
            vm, RT_STDGFX_DRAWRECT_COLOR_INVALID_TYPE, ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    double wVal = ValueAsNum(rawW);
    double hVal = ValueAsNum(rawH);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_DRAWRECT_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }

    GfxDrawRect(gcore, (int)xVal, (int)yVal, (int)wVal, (int)hVal, clr);
    return MakeNil();
}

static PValue gfx_DrawRectangleLines(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawH = args[2];
    PValue rawW = args[3];
    PValue rawThick = args[4];
    PValue rawColor = args[5];

    if (!IsValueNum(rawX)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_X_INVALID_TYPE, ValueTypeToStr(rawX)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_Y_INVALID_TYPE, ValueTypeToStr(rawY)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawW)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_W_INVALID_TYPE, ValueTypeToStr(rawW)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawH)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_H_INVALID_TYPE, ValueTypeToStr(rawH)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawThick)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_THICK_INVALID_TYPE,
            ValueTypeToStr(rawThick)
        );
        return MakeNil();
    }

    if (!IsValueObjType(rawColor, OT_STR)) {
        VmError(
            vm, RT_STDGFX_DRAWRECTLINE_COLOR_INVALID_TYPE,
            ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    double wVal = ValueAsNum(rawW);
    double hVal = ValueAsNum(rawH);
    double thickVal = ValueAsNum(rawThick);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;

    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_DRAWRECTLINE_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }

    GfxDrawRectLine(
        gcore, (int)xVal, (int)yVal, (int)wVal, (int)hVal, (int)thickVal, clr
    );
    return MakeNil();
}

static PValue gfx_DrawCircle(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawR = args[2];
    PValue rawColor = args[3];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_CIRCLE_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_CIRCLE_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }

    if (!IsValueNum(rawR)) {
        VmError(vm, RT_STDGFX_CIRCLE_R_INVALID_TYPE, ValueTypeToStr(rawR));
        return MakeNil();
    }

    if (!IsValueObjType(rawColor, OT_STR)) {
        VmError(
            vm, RT_STDGFX_CIRCLE_COLOR_INVALID_TYPE, ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    double rVal = ValueAsNum(rawR);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;

    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_CIRCLE_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }
    GfxDrawCircle(
        gcore, (int)xVal, (int)yVal, (float)rVal, clr
    ); // BUG: Int Overflow
    return MakeNil();
}

static PValue gfx_DrawCircleLines(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawR = args[2];
    PValue rawThick = args[3];
    PValue rawColor = args[4];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_CIRCLELINE_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_CIRCLELINE_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }

    if (!IsValueNum(rawR)) {
        VmError(vm, RT_STDGFX_CIRCLELINE_R_INVALID_TYPE, ValueTypeToStr(rawR));
        return MakeNil();
    }

    if (!IsValueNum(rawThick)) {
        VmError(
            vm, RT_STDGFX_CIRCLELINE_THICK_INVALID_TYPE,
            ValueTypeToStr(rawThick)
        );
        return MakeNil();
    }

    if (!IsValueObj(rawColor)) {
        VmError(
            vm, RT_STDGFX_CIRCLELINE_COLOR_INVALID_TYPE,
            ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    double rVal = ValueAsNum(rawR);
    double thickVal = ValueAsNum(rawThick);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;

    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_CIRCLELINE_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }
    GfxDrawCircleLine(
        gcore, (int)xVal, (int)yVal, (int)rVal, (int)thickVal, clr
    );

    return MakeNil();
}

static PValue gfx_DrawText(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawText = args[2];
    PValue rawSize = args[3];
    PValue rawColor = args[4];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_TEXT_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_TEXT_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }

    if (!IsValueObjType(rawText, OT_STR)) {
        VmError(vm, RT_STDGFX_TEXT_TEXT_INVALID_TYPE, ValueTypeToStr(rawText));
        return MakeNil();
    }

    if (!IsValueObjType(rawColor, OT_STR)) {
        VmError(
            vm, RT_STDGFX_TEXT_COLOR_INVALID_TYPE, ValueTypeToStr(rawColor)
        );
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    char *text = ValueAsObj(rawText)->v.OString.value;
    double sizeVal = ValueAsNum(rawSize);
    char *colorStr = ValueAsObj(rawColor)->v.OString.value;

    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, RT_STDGFX_TEXT_COLOR_INVALID_VALUE, colorStr);
        return MakeNil();
    }
    GfxDrawText(gcore, xVal, yVal, text, (int)sizeVal, clr);
    return MakeNil();
}
static PValue gfx_Clear(PVm *vm, PValue *args, u64 argc) {
    tigrClear(gcore->screen, GFX_COLOR_WHITE_CODE);
    return MakeNil();
}

static PValue gfx_KeyPress(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];

    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(
            vm, RT_STDGFX_KEYPRESS_KEY_INVALID_TYPE, ValueTypeToStr(rawKey)
        );
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, RT_STDGFX_KEYPRESS_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }
    return MakeBool(GfxKeyPressed(gcore, kbKey));
}

static PValue gfx_KeyDown(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];

    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(vm, RT_STDGFX_KEYDOWN_KEY_INVALID_TYPE, ValueTypeToStr(rawKey));
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, RT_STDGFX_KEYDOWN_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }
    return MakeBool(GfxKeyDown(gcore, kbKey));
}

static PValue gfx_KeyUp(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];

    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(vm, RT_STDGFX_KEYUP_KEY_INVALID_TYPE, ValueTypeToStr(rawKey));
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, RT_STDGFX_KEYUP_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }
    return MakeBool(GfxKeyUp(gcore, kbKey));
}

static PValue gfx_KeyReleased(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];

    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(
            vm, RT_STDGFX_KEYRELEASED_KEY_INVALID_TYPE, ValueTypeToStr(rawKey)
        );
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, RT_STDGFX_KEYRELEASED_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }
    return MakeBool(GfxKeyReleased(gcore, kbKey));
}

static PValue gfx_LoadImage(PVm *vm, PValue *args, u64 argc) {
    PValue rawPath = args[0];

    if (!IsValueObjType(rawPath, OT_STR)) {
        VmError(
            vm, RT_STDGFX_LOADIMG_PATH_INVALID_TYPE, ValueTypeToStr(rawPath)
        );
        return MakeNil();
    }

    char *pathStr = ValueAsObj(rawPath)->v.OString.value;

    if (!DoesFileExists(pathStr)) {
        VmError(vm, RT_TEMPLATE, "Image file cannot be found");
        return MakeNil();
    }
    Tigr *img = tigrLoadImage(pathStr);
    i64 index = GfxCoreAddImage(gcore, img);
    bool ok = false;
    char *str = GfxGetImageString(gcore, index, &ok);
    if (!ok) {
        VmError(vm, RT_IME_STDGFX_LOADIMG_FAILED_FETCH_IMAGE);
        return MakeNil();
    }
    PObj *obj = NewStrObject(vm->gc, NULL, str, true);

    if (obj == NULL) {
        VmError(vm, RT_IME_STDGFX_LOADIMG_FAILED_IMAGE_STR);
        return MakeNil();
    }

    return MakeObject(obj);
}

static PValue gfx_DrawImage(PVm *vm, PValue *args, u64 argc) {
    PValue rawX = args[0];
    PValue rawY = args[1];
    PValue rawImg = args[2];

    if (!IsValueNum(rawX)) {
        VmError(vm, RT_STDGFX_DRAWIMG_X_INVALID_TYPE, ValueTypeToStr(rawX));
        return MakeNil();
    }

    if (!IsValueNum(rawY)) {
        VmError(vm, RT_STDGFX_DRAWIMG_Y_INVALID_TYPE, ValueTypeToStr(rawY));
        return MakeNil();
    }

    if (!IsValueObjType(rawImg, OT_STR)) {
        VmError(vm, RT_STDGFX_DRAWIMG_IMG_INVALID_TYPE, ValueTypeToStr(rawImg));
        return MakeNil();
    }

    double xVal = ValueAsNum(rawX);
    double yVal = ValueAsNum(rawY);
    struct OString *imgStrObj = &ValueAsObj(rawImg)->v.OString;

    i64 index = GfxCoreGetImageIndex(gcore, imgStrObj->value, -1);
    if (index == -1) {
        VmError(vm, RT_STDGFX_DRAWIMG_IMG_INVALID_VALUE);
        return MakeNil();
    }
    bool ok = false;
    Tigr *img = GfxGetImageFromIdx(gcore, index, &ok);
    if (!ok) {
        VmError(vm, RT_IME_STDGFX_DRAWIMG_IMG_FETCH_FAIL);
        return MakeNil();
    }
    tigrBlit(gcore->screen, img, (int)xVal, (int)yVal, 0, 0, img->w, img->h);
    return MakeNil();
}

static PValue gfx_GetMousePos(PVm *vm, PValue *args, u64 argc) {
    double mposX = 0.0;
    double mposY = 0.0;
    GfxGetMousePos(gcore, &mposX, &mposY);
    PValue *arrItems = NULL;
    arrput(arrItems, MakeNumber((double)mposX));
    arrput(arrItems, MakeNumber((double)mposY));
    PObj *arrObj = NewArrayObject(vm->gc, NULL, arrItems, 2);
    arrObj->marked = true;
    return MakeObject(arrObj);
}

static PValue gfx_IsMouseButtonPressed(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];
    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(
            vm, RT_STDGFX_MOUSEPRESSED_KEY_INVALID_TYPE, ValueTypeToStr(rawKey)
        );
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, RT_STDGFX_MOUSEPRESSED_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }

    bool result = GfxMousePressed(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonDown(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];
    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(
            vm, RT_STDGFX_MOUSEDOWN_KEY_INVALID_TYPE, ValueTypeToStr(rawKey)
        );
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, RT_STDGFX_MOUSEDOWN_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }

    bool result = GfxMouseDown(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonReleased(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];
    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(
            vm, RT_STDGFX_MOUSERELEASED_KEY_INVALID_TYPE, ValueTypeToStr(rawKey)
        );
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, RT_STDGFX_MOUSERELEASED_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }

    bool result = GfxMouseReleased(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonUp(PVm *vm, PValue *args, u64 argc) {
    PValue rawKey = args[0];
    if (!IsValueObjType(rawKey, OT_STR)) {
        VmError(vm, RT_STDGFX_MOUSEUP_KEY_INVALID_TYPE, ValueTypeToStr(rawKey));
        return MakeNil();
    }

    char *keyStr = ValueAsObj(rawKey)->v.OString.value;

    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, RT_STDGFX_MOUSEUP_KEY_INVALID_VALUE, keyStr);
        return MakeNil();
    }

    bool result = GfxMouseUp(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_Is2RectCollision(PVm *vm, PValue *args, u64 argc) {
    PValue rawR1x = args[0];
    PValue rawR1y = args[1];
    PValue rawR1h = args[2];
    PValue rawR1w = args[3];

    PValue rawR2x = args[4];
    PValue rawR2y = args[5];
    PValue rawR2h = args[6];
    PValue rawR2w = args[7];

    if (!IsValueNum(rawR1x)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R1X_INVALID_TYPE, ValueTypeToStr(rawR1x)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR1y)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R1Y_INVALID_TYPE, ValueTypeToStr(rawR1y)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR1h)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R1H_INVALID_TYPE, ValueTypeToStr(rawR1h)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR1w)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R1W_INVALID_TYPE, ValueTypeToStr(rawR1w)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR2x)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R2X_INVALID_TYPE, ValueTypeToStr(rawR2x)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR2y)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R2Y_INVALID_TYPE, ValueTypeToStr(rawR2y)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR2h)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R2H_INVALID_TYPE, ValueTypeToStr(rawR2h)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawR2w)) {
        VmError(
            vm, RT_STDGFX_2RECTCOLS_R2W_INVALID_TYPE, ValueTypeToStr(rawR2w)
        );
        return MakeNil();
    }

    double r1x = ValueAsNum(rawR1x);
    double r1y = ValueAsNum(rawR1y);
    double r1w = ValueAsNum(rawR1w);
    double r1h = ValueAsNum(rawR1h);

    double r2x = ValueAsNum(rawR2x);
    double r2y = ValueAsNum(rawR2y);
    double r2w = ValueAsNum(rawR2w);
    double r2h = ValueAsNum(rawR2h);

    bool collide = Gfx2RectColsn(gcore, r1x, r1y, r1w, r1h, r2x, r2y, r2w, r2h);

    return MakeBool(collide);
}

static PValue gfx_IsPointRectCollision(PVm *vm, PValue *args, u64 argc) {
    PValue rawPx = args[0];
    PValue rawPy = args[1];

    PValue rawRx = args[2];
    PValue rawRy = args[3];
    PValue rawRh = args[4];
    PValue rawRw = args[5];

    if (!IsValueNum(rawPx)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_PX_INVALID_TYPE, ValueTypeToStr(rawPx)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawPy)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_PY_INVALID_TYPE, ValueTypeToStr(rawPy)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRx)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_RX_INVALID_TYPE, ValueTypeToStr(rawRx)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRy)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_RY_INVALID_TYPE, ValueTypeToStr(rawRy)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRh)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_RH_INVALID_TYPE, ValueTypeToStr(rawRh)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRw)) {
        VmError(
            vm, RT_STDGFX_POINTRECTCOLS_RW_INVALID_TYPE, ValueTypeToStr(rawRw)
        );
        return MakeNil();
    }

    double px = ValueAsNum(rawPx);
    double py = ValueAsNum(rawPy);

    double rx = ValueAsNum(rawRx);
    double ry = ValueAsNum(rawRy);
    double rw = ValueAsNum(rawRw);
    double rh = ValueAsNum(rawRh);

    bool collide = GfxPointRectColsn(gcore, px, py, rx, ry, rw, rh);

    return MakeBool(collide);
}

static PValue gfx_IsCircleRectCollision(PVm *vm, PValue *args, u64 argc) {
    PValue rawCx = args[0];
    PValue rawCy = args[1];
    PValue rawCr = args[2];

    PValue rawRx = args[3];
    PValue rawRy = args[4];
    PValue rawRh = args[5];
    PValue rawRw = args[6];

    if (!IsValueNum(rawCx)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_CX_INVALID_TYPE, ValueTypeToStr(rawCx)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawCy)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_CY_INVALID_TYPE, ValueTypeToStr(rawCy)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawCr)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_CR_INVALID_TYPE, ValueTypeToStr(rawCr)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRx)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_RX_INVALID_TYPE, ValueTypeToStr(rawRx)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRy)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_RY_INVALID_TYPE, ValueTypeToStr(rawRy)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRh)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_RH_INVALID_TYPE, ValueTypeToStr(rawRh)
        );
        return MakeNil();
    }

    if (!IsValueNum(rawRw)) {
        VmError(
            vm, RT_STDGFX_CIRCLERECTCOLS_RW_INVALID_TYPE, ValueTypeToStr(rawRw)
        );
        return MakeNil();
    }

    double cx = ValueAsNum(rawCx);
    double cy = ValueAsNum(rawCy);
    double cr = ValueAsNum(rawCr);

    double rx = ValueAsNum(rawRx);
    double ry = ValueAsNum(rawRy);
    double rw = ValueAsNum(rawRw);
    double rh = ValueAsNum(rawRh);

    bool collide = GfxCircleRectColsn(gcore, cx, cy, cr, rx, ry, rw, rh);

    return MakeBool(collide);
}

static PValue gfx_GetDelta(PVm *vm, PValue *args, u64 argc) {
    // delta time
    return MakeNumber((double)deltaTime);
}

#define GFX_STD_NEW                 "নতুন"
#define GFX_STD_STOP                "বন্ধ"
#define GFX_STD_RUNNING             "চলমান"
#define GFX_STD_DRAWSTART           "আঁকা_শুরু"
#define GFX_STD_DRAWFINISH          "আঁকা_শেষ"
#define GFX_STD_LINE                "রেখা"
#define GFX_STD_PIXEL               "বিন্দু"
#define GFX_STD_RECT                "আয়তক্ষেত্র"
#define GFX_STD_RECT_LINE           "আয়তক্ষেত্র_রেখা"
#define GFX_STD_CIRCLE              "বৃত্ত"
#define GFX_STD_CIRCLE_LINE         "বৃত্ত_রেখা"
#define GFX_STD_CLEAR               "পরিষ্কার"
#define GFX_STD_TEXT                "লেখা"
#define GFX_STD_PRESSED             "বোতাম_চাপা"
#define GFX_STD_DOWN                "বোতাম_চাপা_আছে"
#define GFX_STD_RELEASED            "বোতাম_ছাড়া"
#define GFX_STD_UP                  "বোতাম_ছাড়া_আছে"
#define GFX_STD_LOAD_IMAGE          "ছবি_আনয়ন"
#define GFX_STD_DRAW_IMAGE          "ছবি_আঁকো"
#define GFX_STD_MOUSE               "মাউস_অবস্থান"
#define GFX_STD_MOUSE_PRESSED       "মাউস_চাপা"
#define GFX_STD_MOUSE_DOWN          "মাউস_চাপা_আছে"
#define GFX_STD_MOUSE_RELEASED      "মাউস_ছাড়া"
#define GFX_STD_MOUSE_UP            "মাউস_ছাড়া_আছে"

#define GFX_STD_COLLIDE_RECT        "স্পর্শ_আয়তক্ষেত্র"
#define GFX_STD_COLLIDE_POINTRECT   "স্পর্শ_বিন্দু_আয়তক্ষেত্র"
#define GFX_STD_COLLIDE_CIRCLE_RECT "স্পর্শ_বৃত্ত_আয়তক্ষেত্র"
#define GFX_STD_DELTA               "ডেল্টা"

void PushStdlibGraphics(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(GFX_STD_NEW, gfx_New, 3),
        MakeStdlibEntry(GFX_STD_STOP, gfx_Stop, 0),
        MakeStdlibEntry(GFX_STD_RUNNING, gfx_Running, 0),
        MakeStdlibEntry(GFX_STD_DRAWSTART, gfx_DrawStart, 0),
        MakeStdlibEntry(GFX_STD_DRAWFINISH, gfx_DrawFinish, 0),
        MakeStdlibEntry(GFX_STD_LINE, gfx_DrawLine, 5),
        MakeStdlibEntry(GFX_STD_PIXEL, gfx_DrawPixel, 3),
        MakeStdlibEntry(GFX_STD_RECT, gfx_DrawRectangle, 5),
        MakeStdlibEntry(GFX_STD_RECT_LINE, gfx_DrawRectangleLines, 6),
        MakeStdlibEntry(GFX_STD_CIRCLE, gfx_DrawCircle, 4),
        MakeStdlibEntry(GFX_STD_CIRCLE_LINE, gfx_DrawCircleLines, 5),
        MakeStdlibEntry(GFX_STD_CLEAR, gfx_Clear, 0),
        MakeStdlibEntry(GFX_STD_TEXT, gfx_DrawText, 5),
        MakeStdlibEntry(GFX_STD_PRESSED, gfx_KeyPress, 1),
        MakeStdlibEntry(GFX_STD_DOWN, gfx_KeyDown, 1),
        MakeStdlibEntry(GFX_STD_RELEASED, gfx_KeyReleased, 1),
        MakeStdlibEntry(GFX_STD_UP, gfx_KeyUp, 1),
        MakeStdlibEntry(GFX_STD_LOAD_IMAGE, gfx_LoadImage, 1),
        MakeStdlibEntry(GFX_STD_DRAW_IMAGE, gfx_DrawImage, 3),
        MakeStdlibEntry(GFX_STD_MOUSE, gfx_GetMousePos, 0),
        MakeStdlibEntry(GFX_STD_MOUSE_PRESSED, gfx_IsMouseButtonPressed, 1),
        MakeStdlibEntry(GFX_STD_MOUSE_DOWN, gfx_IsMouseButtonDown, 1),
        MakeStdlibEntry(GFX_STD_MOUSE_RELEASED, gfx_IsMouseButtonReleased, 1),
        MakeStdlibEntry(GFX_STD_MOUSE_UP, gfx_IsMouseButtonUp, 1),
        MakeStdlibEntry(GFX_STD_COLLIDE_RECT, gfx_Is2RectCollision, 8),
        MakeStdlibEntry(GFX_STD_COLLIDE_POINTRECT, gfx_IsPointRectCollision, 6),
        MakeStdlibEntry(
            GFX_STD_COLLIDE_CIRCLE_RECT, gfx_IsCircleRectCollision, 7
        ),
        MakeStdlibEntry(GFX_STD_DELTA, gfx_GetDelta, 0)
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, GFX_STDLIB_NAME, entries, count);
}
