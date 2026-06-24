/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_GFX_CORE_H
#define PANKTI_GFX_CORE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "external/kb/kb_text_shape.h"
#include "external/stb/stb_truetype.h"
#include "external/tigr/tigr.h"
#include "ptypes.h"
#include <stdbool.h>

#define DEFAULT_STDGFX_WIN_WIDTH  640
#define DEFAULT_STDGFX_WIN_HEIGHT 480
#define DEFAULT_STDGFX_WIN_TITLE  "Pankti Graphics"
#define DEFAULT_STDGFX_FONT_SIZE  PANKB_DEFAULT_FONT_SIZE

#define GFX_CORE_MAX_KEYS         256

#define PANKB_DEFAULT_FONT_SIZE   28

typedef TPixel PColor;
typedef TKey PKey;
typedef struct PanKbCtx PanKbCtx;

typedef struct PanGfxCore {
    // Screen frame buffer (if any)
    Tigr *screen;
    // Screen window handle (if any)
    void *win;
    // Font Context
    PanKbCtx *fontCtx;

    // Image/Sprite List
    Tigr **imageList;

    // Window Width
    int winWidth;

    // Window Height
    int winHeight;

    // Window Title
    char winTitle[1024];

    // Check if window (should be) running
    bool winRunning;

    // everything ready to go
    bool initd;

    int mousePrevBtn;
    int mouseNewPress;
    int mouseNewRelease;

    char kbPrevKeys[GFX_CORE_MAX_KEYS];
    char kbPressedKey[GFX_CORE_MAX_KEYS];
    char kbReleasedKey[GFX_CORE_MAX_KEYS];

} PanGfxCore;

// Drawing+Text Shaping Integration Context
typedef struct PanKbCtx {
    PanGfxCore *core;
    // Font Size to render the text At
    int fontSize;
    // Font file binary data
    unsigned char *fontFileData;
    // File size of font file
    size_t fontFileSize;

    // kb_text_shape context
    kbts_shape_context *kbCtx;

    // stb_truetype font
    stbtt_fontinfo sFontInfo;
} PanKbCtx;

typedef enum ColorStrError {
    CLRSTR_OK = 0,
    CLRSTR_PREFIX_NOT_FOUND,
    CLRSTR_INVALID_COLOR_VAL_R,
    CLRSTR_INVALID_COLOR_VAL_G,
    CLRSTR_INVALID_COLOR_VAL_B,
    CLRSTR_INVALID_COLOR_VAL_A,
    CLRSTR_TOO_LESS_VALS,
    CLRSTR_TOO_MUCH_VALS,
    CLRSTR_UNKNOWN_CLR_NAME,
} ColorStrError;

//-----------------------------------------------------------------------------
// GFX Core Functions
//-----------------------------------------------------------------------------

// Create new GFX Core
PanGfxCore *NewGfxCore(int width, int height, const char *title, int fontSize);
// Free GFX Core
void FreeGfxCore(PanGfxCore *core);

// Make required preparation for starting graphics operations ie. create window
bool StartGfxProcess(PanGfxCore *core);

// Make required preparation for ending graphics operations. Close windows and
// do other stuff
bool EndGfxProcess(PanGfxCore *core);

// Update internal states when called per frame
bool UpdateGfxStatus(PanGfxCore *core);

// Set Icon for GFX Window
void GfxSetWindowIcon(PanGfxCore *core);

//-----------------------------------------------------------------------------
// GFX Image Handling
//-----------------------------------------------------------------------------

// Add Image to Image List
i64 GfxCoreAddImage(PanGfxCore *core, Tigr *img);

// Given the string get image index
i64 GfxCoreGetImageIndex(const PanGfxCore *core, const char *str, i64 len);

// Fetch Image from Core via index
Tigr *GfxGetImageFromIdx(const PanGfxCore *core, i64 index, bool *ok);

// Get Image String from index
char *GfxGetImageString(const PanGfxCore *core, i64 index, bool *ok);

//-----------------------------------------------------------------------------
// GFX Drawing Functions
//-----------------------------------------------------------------------------

bool GfxDrawText(
    PanGfxCore *core, int x, int y, const char *txt, int size, PColor clr
);
bool GfxDrawLine(
    PanGfxCore *core, int x1, int y1, int x2, int y2, int thick, PColor clr
);
bool GfxDrawPixel(PanGfxCore *core, int x1, int y1, PColor clr);
bool GfxDrawRect(PanGfxCore *core, int x, int y, int w, int h, PColor clr);
bool GfxDrawRectLine(
    PanGfxCore *core, int x, int y, int w, int h, int thick, PColor clr
);
bool GfxDrawCircle(PanGfxCore *core, int x, int y, int r, PColor clr);
bool GfxDrawCircleLine(
    PanGfxCore *core, int cx, int cy, int r, int thick, PColor clr
);

//-----------------------------------------------------------------------------
// GFX Input Handling (Mouse & Keyboard)
//-----------------------------------------------------------------------------

bool GfxKeyPressed(PanGfxCore *core, PKey key);
bool GfxKeyDown(PanGfxCore *core, PKey key);
bool GfxKeyReleased(PanGfxCore *core, PKey key);
bool GfxKeyUp(PanGfxCore *core, PKey key);

bool GfxMousePressed(PanGfxCore *core, int key);
bool GfxMouseDown(PanGfxCore *core, int key);
bool GfxMouseReleased(PanGfxCore *core, int key);
bool GfxMouseUp(PanGfxCore *core, int key);

void GfxGetMousePos(PanGfxCore *core, double *xpos, double *ypos);

//-----------------------------------------------------------------------------
// GFX Collison Detection
//-----------------------------------------------------------------------------

bool Gfx2RectColsn(
    PanGfxCore *core,
    int x1,
    int y1,
    int w1,
    int h1,
    int x2,
    int y2,
    int w2,
    int h2
);
bool GfxPointRectColsn(
    PanGfxCore *core, int px, int py, int rx, int ry, int rw, int rh
);

bool GfxCircleRectColsn(
    PanGfxCore *core, int cx, int cy, int cr, int rx, int ry, int rw, int rh
);

//-----------------------------------------------------------------------------
// GFX Color Helpers
//-----------------------------------------------------------------------------

PColor ParseColorString(const char *str, ColorStrError *err);

PColor PanStrToColor(const char *str, ColorStrError *err);
// Image LoadGuiAppIcon(void);

PKey PanStrToKeyboardKey(const char *keyStr, i64 len);
int PanStrToMouseKey(const char *keyStr, i64 len);

//-----------------------------------------------------------------------------
// GFX Font & Text Shaping
//-----------------------------------------------------------------------------

// Create a new Raylib+kb_text_shape integration context Font Context
PanKbCtx *NewPanKbCtxFontContext(void);
// Free Raylib+kb_text_shape Font Context
void FreePanKbCtxFontContext(PanKbCtx *ctx);
// Load a font file. Resets previous states and buffer text
bool PanKbCtxLoadFontFile(PanKbCtx *ctx, const char *filepath);
// Draw text with previously loaded font from context
bool PanKbCtxDrawText(
    PanKbCtx *ctx, int xpos, int ypos, const char *text, PColor color
);
// Load default font (Noto Serif Bengali Regular)
bool PanKbLoadDefaultNotoFont(PanKbCtx *ctx);

#ifdef __cplusplus
}
#endif

#endif
