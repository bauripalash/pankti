#include "../external/stb/stb_ds.h"
#include "../external/tigr/tigr.h"
#include "../include/gc.h"
#include "../include/gfxcore.h"
#include "../include/gfxdraw.h"
#include "../include/gfxfont.h"
#include "../include/gfxhelper.h"
#include "../include/pstdlib.h"
#include "../include/symtable.h"
#include "../include/vm.h"
#include <stdbool.h>

static PanGfxCore *gcore = NULL;
float deltaTime = 0.0;

static PValue gfx_New(PVm *vm, PValue *args, u64 argc) {
    double winW = ValueAsNum(args[0]);
    double winH = ValueAsNum(args[1]);
    char *title = ValueAsObj(args[2])->v.OString.value;
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
    return MakeNil();
}

static PValue gfx_DrawLine(PVm *vm, PValue *args, u64 argc) {
    double x1Val = ValueAsNum(args[0]);
    double y1Val = ValueAsNum(args[1]);
    double x2Val = ValueAsNum(args[2]);
    double y2Val = ValueAsNum(args[3]);

    char *colorStr = ValueAsObj(args[4])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }

    GfxDrawLine(gcore, (int)x1Val, (int)y1Val, (int)x2Val, (int)y2Val, 1, clr);

    return MakeNil();
}

static PValue gfx_DrawPixel(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    char *colorStr = ValueAsObj(args[2])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }
    GfxDrawPixel(gcore, (int)xVal, (int)yVal, clr);
    return MakeNil();
}

static PValue gfx_DrawRectangle(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double wVal = ValueAsNum(args[2]);
    double hVal = ValueAsNum(args[3]);
    char *colorStr = ValueAsObj(args[4])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }

    GfxDrawRect(gcore, (int)xVal, (int)yVal, (int)wVal, (int)hVal, clr);
    return MakeNil();
}

static PValue gfx_DrawRectangleLines(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double wVal = ValueAsNum(args[2]);
    double hVal = ValueAsNum(args[3]);
    double thickVal = ValueAsNum(args[4]);
    char *colorStr = ValueAsObj(args[5])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }

    GfxDrawRectLine(
        gcore, (int)xVal, (int)yVal, (int)wVal, (int)hVal, (int)thickVal, clr
    );
    return MakeNil();
}

static PValue gfx_DrawCircle(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double rVal = ValueAsNum(args[2]);
    char *colorStr = ValueAsObj(args[3])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }
    GfxDrawCircle(gcore, (int)xVal, (int)yVal, (float)rVal, clr);
    return MakeNil();
}

static PValue gfx_DrawCircleLines(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double rVal = ValueAsNum(args[2]);
    double thickVal = ValueAsNum(args[3]);
    char *colorStr = ValueAsObj(args[4])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
        return MakeNil();
    }
    GfxDrawCircleLine(
        gcore, (int)xVal, (int)yVal, (int)rVal, (int)thickVal, clr
    );

    return MakeNil();
}

static PValue gfx_DrawText(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    char *text = ValueAsObj(args[2])->v.OString.value;
    double sizeVal = ValueAsNum(args[3]);
    char *colorStr = ValueAsObj(args[4])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    PColor clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        VmError(vm, "Invalid Color");
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
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, "Invalid key");
        return MakeNil();
    }
    return MakeBool(GfxKeyPressed(gcore, kbKey));
}

static PValue gfx_KeyDown(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, "Invalid key");
        return MakeNil();
    }
    return MakeBool(GfxKeyDown(gcore, kbKey));
}

static PValue gfx_KeyUp(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, "Invalid key");
        return MakeNil();
    }
    return MakeBool(GfxKeyUp(gcore, kbKey));
}

static PValue gfx_KeyReleased(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    PKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == 0) {
        VmError(vm, "Invalid key");
        return MakeNil();
    }
    return MakeBool(GfxKeyReleased(gcore, kbKey));
}

static PValue gfx_LoadImage(PVm *vm, PValue *args, u64 argc) {
    char *pathStr = ValueAsObj(args[0])->v.OString.value;
    if (!DoesFileExists(pathStr)) {
        VmError(vm, "Image file cannot be found");
        return MakeNil();
    }
    Tigr *img = tigrLoadImage(pathStr);
    i64 index = GfxCoreAddImage(gcore, img);
    bool ok = false;
    char *str = GfxGetImageString(gcore, index, &ok);
    if (!ok) {
        VmError(vm, "Internal Error : Failed to load image");
        return MakeNil();
    }
    PObj *obj = NewStrObject(vm->gc, NULL, str, true);

    if (obj == NULL) {
        VmError(vm, "Internal Error : Failed to load image");
        return MakeNil();
    }

    return MakeObject(obj);
}

static PValue gfx_DrawImage(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    struct OString *imgStrObj = &ValueAsObj(args[2])->v.OString;

    i64 index = GfxCoreGetImageIndex(gcore, imgStrObj->value, -1);
    if (index == -1) {
        VmError(vm, "Invalid image to draw");
        return MakeNil();
    }
    bool ok = false;
    Tigr *img = GfxGetImageFromIdx(gcore, index, &ok);
    if (!ok) {
        VmError(vm, "Invalid image to draw");
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
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, "Invalid mouse key name");
        return MakeNil();
    }

    bool result = GfxMousePressed(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonDown(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, "Invalid mouse key name");
        return MakeNil();
    }

    bool result = GfxMouseDown(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonReleased(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, "Invalid mouse key name");
        return MakeNil();
    }

    bool result = GfxMouseReleased(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonUp(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        VmError(vm, "Invalid mouse key name");
        return MakeNil();
    }

    bool result = GfxMouseUp(gcore, btnInt);
    return MakeBool(result);
}

static PValue gfx_Is2RectCollision(PVm *vm, PValue *args, u64 argc) {
    double r1x = ValueAsNum(args[0]);
    double r1y = ValueAsNum(args[1]);
    double r1w = ValueAsNum(args[2]);
    double r1h = ValueAsNum(args[3]);

    double r2x = ValueAsNum(args[4]);
    double r2y = ValueAsNum(args[5]);
    double r2w = ValueAsNum(args[6]);
    double r2h = ValueAsNum(args[7]);

    bool collide = Gfx2RectColsn(gcore, r1x, r1y, r1w, r1h, r2x, r2y, r2w, r2h);

    return MakeBool(collide);
}

static PValue gfx_IsPointRectCollision(PVm *vm, PValue *args, u64 argc) {
    double px = ValueAsNum(args[0]);
    double py = ValueAsNum(args[1]);

    double rx = ValueAsNum(args[2]);
    double ry = ValueAsNum(args[3]);
    double rw = ValueAsNum(args[4]);
    double rh = ValueAsNum(args[5]);

    bool collide = GfxPointRectColsn(gcore, px, py, rx, ry, rw, rh);

    return MakeBool(collide);
}

static PValue gfx_GetDelta(PVm *vm, PValue *args, u64 argc) {
    // delta time
    return MakeNumber((double)deltaTime);
}

#define GFX_STD_NEW               "নতুন"
#define GFX_STD_STOP              "বন্ধ"
#define GFX_STD_RUNNING           "চলমান"
#define GFX_STD_DRAWSTART         "আঁকা_শুরু"
#define GFX_STD_DRAWFINISH        "আঁকা_শেষ"
#define GFX_STD_LINE              "রেখা"
#define GFX_STD_PIXEL             "বিন্দু"
#define GFX_STD_RECT              "আয়তক্ষেত্র"
#define GFX_STD_RECT_LINE         "আয়তক্ষেত্র_রেখা"
#define GFX_STD_CIRCLE            "বৃত্ত"
#define GFX_STD_CIRCLE_LINE       "বৃত্ত_রেখা"
#define GFX_STD_CLEAR             "পরিষ্কার"
#define GFX_STD_TEXT              "লেখা"
#define GFX_STD_PRESSED           "বোতাম_চাপা"
#define GFX_STD_DOWN              "বোতাম_চাপা_আছে"
#define GFX_STD_RELEASED          "বোতাম_ছাড়া"
#define GFX_STD_UP                "বোতাম_ছাড়া_আছে"
#define GFX_STD_LOAD_IMAGE        "ছবি_আনয়ন"
#define GFX_STD_DRAW_IMAGE        "ছবি_আঁকো"
#define GFX_STD_MOUSE             "মাউস_অবস্থান"
#define GFX_STD_MOUSE_PRESSED     "মাউস_চাপা"
#define GFX_STD_MOUSE_DOWN        "মাউস_চাপা_আছে"
#define GFX_STD_MOUSE_RELEASED    "মাউস_ছাড়া"
#define GFX_STD_MOUSE_UP          "মাউস_ছাড়া_আছে"

#define GFX_STD_COLLIDE_RECT      "স্পর্শ_আয়তক্ষেত্র"
#define GFX_STD_COLLIDE_POINTRECT "স্পর্শ_বিন্দু_আয়তক্ষেত্র"
#define GFX_STD_DELTA             "ডেল্টা"

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
        MakeStdlibEntry(GFX_STD_DELTA, gfx_GetDelta, 0)
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, GFX_STDLIB_NAME, entries, count);
}
