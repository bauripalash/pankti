#ifndef PANKTI_GFX_CORE_H
#define PANKTI_GFX_CORE_H

#include "ptypes.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "../external/raylib/raylib.h"
#include "gfxfont.h"

#define DEFAULT_STDGFX_WIN_WIDTH  640
#define DEFAULT_STDGFX_WIN_HEIGHT 480
#define DEFAULT_STDGFX_WIN_TITLE  "Pankti Graphics"
#define DEFAULT_STDGFX_FONT_SIZE  PANKB_DEFAULT_FONT_SIZE

typedef struct PanGfxCore {
    // Screen frame buffer (if any)
    void *screen;
    // Screen window handle (if any)
    void *win;
    // Font Context
    PanKbCtx *fontCtx;

    // Image/Sprite List
    Image *imageList;

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

} PanGfxCore;

// Create new gfx core
PanGfxCore *NewGfxCore(int width, int height, const char *title, int fontSize);
// Free gfx core
void FreeGfxCore(PanGfxCore *core);

// Make required preparation for starting graphics operations ie. create windo
bool StartGfxProcess(PanGfxCore *core);

// Make required preparation for ending graphics operations. Close windows and
// do other stuff
bool EndGfxProcess(PanGfxCore *core);

bool UpdateGfxStatus(PanGfxCore *core);
// Image handling

// Add Image to Image List
i64 GfxCoreAddImage(PanGfxCore *core, Image img);

// Given the string get image index
i64 GfxCoreGetImageIndex(const PanGfxCore *core, const char *str, i64 len);

// Fetch Image from Core via index
Image GfxGetImageFromIdx(const PanGfxCore *core, i64 index, bool *ok);

// Get Image String from index
char *GfxGetImageString(const PanGfxCore *core, i64 index, bool *ok);
#ifdef __cplusplus
}
#endif

#endif
