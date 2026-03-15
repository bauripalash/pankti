#include "../include/gfxdraw.h"
#include "../external/tigr/tigr.h"
#include <stdbool.h>
#include <stdlib.h>

static void drawpx(PanGfxCore *core, int x, int y, int size, PColor clr) {
    tigrFillRect(core->screen, x, y, size, size, clr);
}

bool GfxDrawText(PanGfxCore *core, int x, int y, const char *txt, PColor clr) {
    if (core == NULL || core->fontCtx == NULL) {
        return false;
    }

    PanKbCtxDrawText(core->fontCtx, x, y, txt, clr);
    return true;
}

bool GfxDrawLine(
    PanGfxCore *core, int x1, int y1, int x2, int y2, int thick, PColor clr
) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2 = 0;

    while (true) {
        drawpx(core, x1, y1, thick, clr);
        e2 = 2 * err;

        if (x1 == x2 && y1 == y2) {
            break;
        }

        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }

    return true;
}
bool GfxDrawPixel(PanGfxCore *core, int x1, int y1, PColor clr) {
    drawpx(core, x1, y1, 1, clr);
    return true;
}
bool GfxDrawRect(PanGfxCore *core, int x, int y, int w, int h, PColor clr) {
    // DrawRectangle(x, y, w, h, clr);
    tigrFillRect(core->screen, x, y, w, h, clr);
    return true;
}
bool GfxDrawRectLine(
    PanGfxCore *core, int x, int y, int w, int h, int thick, PColor clr
) {
    // DrawRectangleLinesEx((Rectangle){x, y, w, h}, thick, clr);
    tigrRect(core->screen, x, y, w, h, clr);
    return true;
}
bool GfxDrawCircle(PanGfxCore *core, int x, int y, int r, PColor clr) {
    // DrawCircle(x, y, r, clr);
    tigrFillCircle(core->screen, x, y, r, clr);
    return true;
}
bool GfxDrawCircleLine(
    PanGfxCore *core, int cx, int cy, int r, int thick, PColor clr
) {
    int x = -r;
    int y = 0;
    int err = 2 - 2 * r;

    do {
        tigrPlot(core->screen, cx - x, cy + y, clr);
        tigrPlot(core->screen, cx - y, cy - x, clr);
        tigrPlot(core->screen, cx + x, cy - y, clr);
        tigrPlot(core->screen, cx + y, cy + x, clr);

        r = err;
        if (r <= y) {
            err += ++y * 2 + 1;
        }
        if (r > x || err > y) {
            err += ++x * 2 + 1;
        }
    } while (x < 0);

    return true;
}

bool GfxKeyPressed(PanGfxCore *core, PKey key) {
    return tigrKeyDown(core->screen, key) != 0;
}
bool GfxKeyDown(PanGfxCore *core, PKey key) {
    return tigrKeyHeld(core->screen, key) != 0;
}
bool GfxKeyReleased(PanGfxCore *core, PKey key) { return false; }
bool GfxKeyUp(PanGfxCore *core, PKey key) { return false; }

bool GfxMousePressed(PanGfxCore *core, int key) {
    int x = 0;
    int y = 0;
    int btn = 0;
    tigrMouse(core->screen, &x, &y, &btn);
    return (btn & key);
    // tigrMouse(Tigr *bmp, int *x, int *y, int *buttons);
    // return IsMouseButtonPressed(key);
}
bool GfxMouseDown(PanGfxCore *core, int key) {
    int x;
    int y;
    int btn;
    // return IsMouseButtonDown(key);
    tigrMouse(core->screen, &x, &y, &btn);
    return (btn & key);
}
bool GfxMouseReleased(PanGfxCore *core, int key) {
    int x;
    int y;
    int btn;
    // return IsMouseButtonReleased(key);
    return false;
}
bool GfxMouseUp(PanGfxCore *core, int key) {
    int x;
    int y;
    int btn;
    // return IsMouseButtonUp(key);
    return false;
}

void GfxGetMousePos(PanGfxCore *core, double *xpos, double *ypos) {
    int x = 0;
    int y = 0;
    int btn = 0;
    tigrMouse(core->screen, &x, &y, &btn);
    *xpos = (double)x;
    *ypos = (double)y;
}

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
) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}
bool GfxPointRectColsn(
    PanGfxCore *core, int px, int py, int rx, int ry, int rw, int rh
) {
    int x1 = rx;
    int y1 = ry;
    int x2 = rx + rw;
    int y2 = ry + rh;

    return (px > x1 && px < x2 && py > y1 && py < y2);
}
