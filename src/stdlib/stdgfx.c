#include "../external/raylib/raylib.h"
#include "../external/stb/stb_ds.h"
#include "../include/gc.h"
#include "../include/gfxfont.h"
#include "../include/gfxhelper.h"
#include "../include/pstdlib.h"
#include "../include/symtable.h"
#include "../include/vm.h"
#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define DEFAULT_GFX_WIN_WIDTH  640
#define DEFAULT_GFX_WIN_HEIGHT 480
#define DEFAULT_GFX_WIN_TITLE  "Pankti Graphics"
#define DEFAULT_GFX_FONT_SIZE  PANKB_DEFAULT_FONT_SIZE

static PanKbCtx *fontCtx = NULL;
static int winWidth = DEFAULT_GFX_WIN_WIDTH;
static int winHeight = DEFAULT_GFX_WIN_HEIGHT;
static char winTitle[1024];
static bool winRunning = false;
static Image *imageList = NULL;

static void unloadImageList(void) {
    i64 count = arrlen(imageList);
    for (i64 i = count; i > 0; i--) {
        UnloadImage(arrpop(imageList));
    }

    arrfree(imageList);
    imageList = NULL;
}

static u64 addToImageList(Image img) {
    u64 len = arrlen(imageList);
    arrput(imageList, img);
    return len;
}

static i64 getImgIndexFromStr(const char *str, i64 len) {
    u64 slen = 0;
    if (len == -1) {
        slen = StrLength(str);
    } else {
        slen = (u64)len;
    }
    if (StrStartsWith(str, GFX_IMAGE_STR_PREFIX)) {
        const char *ptr = str;

        ptr += sizeof(GFX_IMAGE_STR_PREFIX) - 1;
        bool ok = false;
        double rawNum =
            NumberFromStr(ptr, slen - (sizeof(GFX_IMAGE_STR_PREFIX) - 1), &ok);

        if (IsDoubleInt(rawNum) && rawNum >= 0) {
            return (i64)floor(rawNum);
        }
    }
    return -1;
}

static Image getImageFromIndex(i64 index, bool *ok) {
    if (index < 0 || index >= arrlen(imageList)) {
        *ok = false;
        return (Image){0};
    }

    *ok = true;
    return imageList[index];
}

static char *makeImageStr(u64 index, bool *ok) {
    if (index < 0 || index >= arrlen(imageList)) {
        *ok = false;
        return NULL;
    }

    const char *fmtStr = StrFormat("%s%zu", GFX_IMAGE_STR_PREFIX, index);
    char *result = StrDuplicate(fmtStr, strlen(fmtStr));

    if (result == NULL) {
        *ok = false;
        return NULL;
    }
    *ok = true;
    return result;
}

static void startGfx(void) {
    if (fontCtx == NULL) {
        fontCtx = NewPanKbCtxFontContext();
        fontCtx->fontSize = DEFAULT_GFX_FONT_SIZE;
        PanKbLoadDefaultNotoFont(fontCtx);
    }

    winRunning = true;
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(winWidth, winHeight, winTitle);
    SetTargetFPS(60);
    Image winIcon = LoadGuiAppIcon();
    arrput(imageList, winIcon);
}

static void ensureGfx(void) {
    if (fontCtx == NULL) {
        startGfx();
    }
}

static void updateStatus(void) {
    winRunning = !WindowShouldClose();
    return;
}

static void stopGfx(void) {
    unloadImageList();
    CloseWindow();
}

static PValue gfx_New(PVm *vm, PValue *args, u64 argc) {
    double winW = ValueAsNum(args[0]);
    double winH = ValueAsNum(args[1]);
    char *title = ValueAsObj(args[2])->v.OString.value;
    winWidth = winW;
    winHeight = winH;
    strcpy(winTitle, title);
    startGfx();
    return MakeNil();
}

static PValue gfx_Stop(PVm *vm, PValue *args, u64 argc) {
    stopGfx();
    return MakeNil();
}

static PValue gfx_Running(PVm *vm, PValue *args, u64 argc) {
    ensureGfx();
    updateStatus();
    return MakeBool(winRunning);
}

static PValue gfx_DrawStart(PVm *vm, PValue *args, u64 argc) {
    BeginDrawing();
    return MakeNil();
}

static PValue gfx_DrawFinish(PVm *vm, PValue *args, u64 argc) {
    EndDrawing();
    return MakeNil();
}

static PValue gfx_DrawLine(PVm *vm, PValue *args, u64 argc) {
    double x1Val = ValueAsNum(args[0]);
    double y1Val = ValueAsNum(args[1]);
    double x2Val = ValueAsNum(args[2]);
    double y2Val = ValueAsNum(args[3]);

    char *colorStr = ValueAsObj(args[4])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    Color clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        return MakeError(vm->gc, "Invalid Color");
    }

    DrawLine((int)x1Val, (int)y1Val, (int)x2Val, (int)y2Val, clr);

    return MakeNil();
}

static PValue gfx_DrawPixel(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    char *colorStr = ValueAsObj(args[2])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    Color clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        return MakeError(vm->gc, "Invalid Color");
    }
    DrawPixel((int)xVal, (int)yVal, clr);
    return MakeNil();
}

static PValue gfx_DrawRectangle(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double wVal = ValueAsNum(args[2]);
    double hVal = ValueAsNum(args[3]);
    DrawRectangleRec(
        (Rectangle){(int)xVal, (int)yVal, (int)wVal, (int)hVal},
        GFX_COLOR_BLACK_CODE
    );
    return MakeNil();
}

static PValue gfx_DrawCircle(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    double rVal = ValueAsNum(args[2]);
    char *colorStr = ValueAsObj(args[3])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    Color clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        return MakeError(vm->gc, "Invalid Color");
    }
    DrawCircle((int)xVal, (int)yVal, (float)rVal, clr);
    return MakeNil();
}

static PValue gfx_DrawText(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    char *text = ValueAsObj(args[2])->v.OString.value;
    char *colorStr = ValueAsObj(args[3])->v.OString.value;
    ColorStrError err = CLRSTR_OK;
    Color clr = PanStrToColor(colorStr, &err);
    if (err != CLRSTR_OK) {
        return MakeError(vm->gc, "Invalid Color");
    }
    PanKbCtxDrawText(fontCtx, xVal, yVal, text, clr);
    return MakeNil();
}
static PValue gfx_Clear(PVm *vm, PValue *args, u64 argc) {
    ensureGfx();
    ClearBackground(GFX_COLOR_WHITE_CODE);
    return MakeNil();
}

static PValue gfx_KeyPress(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    KeyboardKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == KEY_NULL) {
        return MakeError(vm->gc, "Invalid key");
    }
    return MakeBool(IsKeyPressed(kbKey));
}

static PValue gfx_KeyDown(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    KeyboardKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == KEY_NULL) {
        return MakeError(vm->gc, "Invalid key");
    }
    return MakeBool(IsKeyDown(kbKey));
}

static PValue gfx_KeyUp(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    KeyboardKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == KEY_NULL) {
        return MakeError(vm->gc, "Invalid key");
    }
    return MakeBool(IsKeyUp(kbKey));
}

static PValue gfx_KeyReleased(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    KeyboardKey kbKey = PanStrToKeyboardKey(keyStr, -1);
    if (kbKey == KEY_NULL) {
        return MakeError(vm->gc, "Invalid key");
    }
    return MakeBool(IsKeyReleased(kbKey));
}

static PValue gfx_LoadImage(PVm *vm, PValue *args, u64 argc) {
    char *pathStr = ValueAsObj(args[0])->v.OString.value;
    if (!FileExists(pathStr)) {
        return MakeError(vm->gc, "Image file cannot be found");
    }
    Image img = LoadImage(pathStr);
    i64 index = addToImageList(img);
    bool ok = false;
    char *str = makeImageStr(index, &ok);
    if (!ok) {
        return MakeError(vm->gc, "Internal Error : Failed to load image");
    }
    PObj *obj = NewStrObject(vm->gc, NULL, str, true);

    if (obj == NULL) {
        return MakeError(vm->gc, "Internal Error : Failed to load image");
    }

    return MakeObject(obj);
}

static PValue gfx_DrawImage(PVm *vm, PValue *args, u64 argc) {
    double xVal = ValueAsNum(args[0]);
    double yVal = ValueAsNum(args[1]);
    struct OString *imgStrObj = &ValueAsObj(args[2])->v.OString;

    i64 index = getImgIndexFromStr(imgStrObj->value, -1);
    if (index == -1) {
        return MakeError(vm->gc, "Invalid image to draw");
    }
    bool ok = false;
    Image img = getImageFromIndex(index, &ok);
    if (!ok) {
        return MakeError(vm->gc, "Invalid image to draw");
    }
    Texture2D imgTxt = LoadTextureFromImage(img);
    DrawTexture(imgTxt, (int)xVal, (int)yVal, WHITE);
    return MakeNil();
}

static PValue gfx_GetMousePos(PVm *vm, PValue *args, u64 argc) {
    Vector2 mpos = GetMousePosition();
    PValue *arrItems = NULL;
    arrput(arrItems, MakeNumber((double)mpos.x));
    arrput(arrItems, MakeNumber((double)mpos.y));
    PObj *arrObj = NewArrayObject(vm->gc, NULL, arrItems, 2);
    arrObj->marked = true;
    return MakeObject(arrObj);
}

static PValue gfx_IsMouseButtonPressed(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        return MakeError(vm->gc, "Invalid mouse key name");
    }

    bool result = IsMouseButtonPressed(btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonDown(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        return MakeError(vm->gc, "Invalid mouse key name");
    }

    bool result = IsMouseButtonDown(btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonReleased(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        return MakeError(vm->gc, "Invalid mouse key name");
    }

    bool result = IsMouseButtonReleased(btnInt);
    return MakeBool(result);
}

static PValue gfx_IsMouseButtonUp(PVm *vm, PValue *args, u64 argc) {
    char *keyStr = ValueAsObj(args[0])->v.OString.value;
    int btnInt = PanStrToMouseKey(keyStr, -1);
    if (btnInt == -1) {
        return MakeError(vm->gc, "Invalid mouse key name");
    }

    bool result = IsMouseButtonUp(btnInt);
    return MakeBool(result);
}

static PValue gfx_Is2RectCollison(PVm *vm, PValue *args, u64 argc) {
    double r1x = ValueAsNum(args[0]);
    double r1y = ValueAsNum(args[1]);
    double r1w = ValueAsNum(args[2]);
    double r1h = ValueAsNum(args[3]);

    double r2x = ValueAsNum(args[4]);
    double r2y = ValueAsNum(args[5]);
    double r2w = ValueAsNum(args[6]);
    double r2h = ValueAsNum(args[7]);

    Rectangle rect1 = (Rectangle){r1x, r1y, r1w, r1h};
    Rectangle rect2 = (Rectangle){r2x, r2y, r2w, r2h};

    bool collide = CheckCollisionRecs(rect1, rect2);

    return MakeBool(collide);
}

static PValue gfx_IsPointRectCollison(PVm *vm, PValue *args, u64 argc) {
    double px = ValueAsNum(args[0]);
    double py = ValueAsNum(args[1]);

    double rx = ValueAsNum(args[2]);
    double ry = ValueAsNum(args[3]);
    double rw = ValueAsNum(args[4]);
    double rh = ValueAsNum(args[5]);

    Vector2 point = (Vector2){px, py};
    Rectangle rect = (Rectangle){rx, ry, rw, rh};

    bool collide = CheckCollisionPointRec(point, rect);

    return MakeBool(collide);
}

static PValue gfx_GetDelta(PVm *vm, PValue *args, u64 argc) {
    return MakeNumber(GetFrameTime());
}

#define GFX_STD_NEW               "নতুন"
#define GFX_STD_STOP              "বন্ধ"
#define GFX_STD_RUNNING           "চলমান"
#define GFX_STD_DRAWSTART         "আঁকা_শুরু"
#define GFX_STD_DRAWFINISH        "আঁকা_শেষ"
#define GFX_STD_LINE              "রেখা"
#define GFX_STD_PIXEL             "বিন্দু"
#define GFX_STD_RECT              "আয়তক্ষেত্র"
#define GFX_STD_CIRCLE            "বৃত্ত"
#define GFX_STD_CLEAR             "পরিষ্কার"
#define GFX_STD_TEXT              "লেখা"
#define GFX_STD_PRESSED           "বোতাম_চাপা"
#define GFX_STD_DOWN              "বোতাম_নীচে"
#define GFX_STD_RELEASED          "বোতাম_ছাড়া"
#define GFX_STD_UP                "বোতাম_উপরে"
#define GFX_STD_LOAD_IMAGE        "ছবি_আনয়ন"
#define GFX_STD_DRAW_IMAGE        "ছবি_আঁকো"
#define GFX_STD_MOUSE             "মাউস_অবস্থান"
#define GFX_STD_MOUSE_PRESSED     "মাউস_চাপা"
#define GFX_STD_MOUSE_DOWN        "মাউস_নীচে"
#define GFX_STD_MOUSE_RELEASED    "মাউস_ছাড়া"
#define GFX_STD_MOUSE_UP          "মাউস_উপরে"

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
        MakeStdlibEntry(GFX_STD_RECT, gfx_DrawRectangle, 4),
        MakeStdlibEntry(GFX_STD_CIRCLE, gfx_DrawCircle, 4),
        MakeStdlibEntry(GFX_STD_CLEAR, gfx_Clear, 0),
        MakeStdlibEntry(GFX_STD_TEXT, gfx_DrawText, 4),
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
        MakeStdlibEntry(GFX_STD_COLLIDE_RECT, gfx_Is2RectCollison, 8),
        MakeStdlibEntry(GFX_STD_COLLIDE_POINTRECT, gfx_IsPointRectCollison, 6),
        MakeStdlibEntry(GFX_STD_DELTA, gfx_GetDelta, 0)
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, entries, count);
}
