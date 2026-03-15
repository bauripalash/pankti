#ifndef PANKTI_GFX_DRAW_H
#define PANKTI_GFX_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gfxcore.h"
#include "gfxhelper.h"
#include <stdbool.h>

bool GfxDrawText(PanGfxCore *core, int x, int y, const char *txt, PColor clr);
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

bool GfxKeyPressed(PanGfxCore *core, PKey key);
bool GfxKeyDown(PanGfxCore *core, PKey key);
bool GfxKeyReleased(PanGfxCore *core, PKey key);
bool GfxKeyUp(PanGfxCore *core, PKey key);

bool GfxMousePressed(PanGfxCore *core, int key);
bool GfxMouseDown(PanGfxCore *core, int key);
bool GfxMouseReleased(PanGfxCore *core, int key);
bool GfxMouseUp(PanGfxCore *core, int key);

void GfxGetMousePos(PanGfxCore *core, double *xpos, double *ypos);

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

#ifdef __cplusplus
}
#endif

#endif
