#include "../external/raylib/raylib.h"
#include "../include/gc.h"
#include "../include/gfxfont.h"
#include "../include/gfxhelper.h"
#include "../include/pstdlib.h"
#include "../include/symtable.h"
#include "../include/vm.h"
#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define DEFAULT_GFX_WIN_WIDTH  640
#define DEFAULT_GFX_WIN_HEIGHT 480
#define DEFAULT_GFX_WIN_TITLE  "Pankti Graphics"
#define DEFAULT_GFX_FONT_SIZE  72

static PanKbCtx *fontCtx = NULL;
static int winWidth = DEFAULT_GFX_WIN_WIDTH;
static int winHeight = DEFAULT_GFX_WIN_HEIGHT;
static char winTitle[1024];
static bool winRunning = false;

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
    LoadGuiAppIcon();
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

static void stopGfx(void) { CloseWindow(); }

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

#define GFX_STD_NEW        "new"
#define GFX_STD_STOP       "stop"
#define GFX_STD_RUNNING    "running"
#define GFX_STD_DRAWSTART  "start"
#define GFX_STD_DRAWFINISH "finish"
#define GFX_STD_PIXEL      "pixel"
#define GFX_STD_RECT       "rect"
#define GFX_STD_CIRCLE     "circle"
#define GFX_STD_CLEAR      "clear"
#define GFX_STD_TEXT       "text"

void PushStdlibGraphics(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(GFX_STD_NEW, gfx_New, 3),
        MakeStdlibEntry(GFX_STD_STOP, gfx_Stop, 0),
        MakeStdlibEntry(GFX_STD_RUNNING, gfx_Running, 0),
        MakeStdlibEntry(GFX_STD_DRAWSTART, gfx_DrawStart, 0),
        MakeStdlibEntry(GFX_STD_DRAWFINISH, gfx_DrawFinish, 0),
        MakeStdlibEntry(GFX_STD_PIXEL, gfx_DrawPixel, 3),
        MakeStdlibEntry(GFX_STD_RECT, gfx_DrawRectangle, 4),
        MakeStdlibEntry(GFX_STD_CIRCLE, gfx_DrawCircle, 4),
        MakeStdlibEntry(GFX_STD_CLEAR, gfx_Clear, 0),
        MakeStdlibEntry(GFX_STD_TEXT, gfx_DrawText, 4),
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, entries, count);
}
