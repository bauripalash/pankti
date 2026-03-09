#include "../include/gfxdraw.h"
#include "../external/raylib/raylib.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

static void drawpx(int x, int y, int size, Color clr) {
    DrawRectangle(x, y, size, size, clr);
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
        drawpx(x1, y1, thick, clr);
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
    drawpx(x1, y1, 1, clr);
    return true;
}
bool GfxDrawRect(PanGfxCore *core, int x, int y, int w, int h, PColor clr) {
    DrawRectangle(x, y, w, h, clr);
    return true;
}
bool GfxDrawRectLine(
    PanGfxCore *core, int x, int y, int w, int h, int thick, PColor clr
) {
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, thick, clr);
    return true;
}
bool GfxDrawCircle(PanGfxCore *core, int x, int y, int r, PColor clr) {
    DrawCircle(x, y, r, clr);
    return true;
}
bool GfxDrawCircleLine(
    PanGfxCore *core, int cx, int cy, int r, int thick, PColor clr
) {
    int x = -r;
    int y = 0;
    int err = 2 - 2 * r;

    do {
        DrawPixel(cx - x, cy + y, clr);
        DrawPixel(cx - y, cy - x, clr);
        DrawPixel(cx + x, cy - y, clr);
        DrawPixel(cx + y, cy + x, clr);

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

bool GfxKeyPressed(PanGfxCore *core, PKey key) { return IsKeyPressed(key); }
bool GfxKeyDown(PanGfxCore *core, PKey key) { return IsKeyDown(key); }
bool GfxKeyReleased(PanGfxCore *core, PKey key) { return IsKeyReleased(key); }
bool GfxKeyUp(PanGfxCore *core, PKey key) { return IsKeyUp(key); }

bool GfxMousePressed(PanGfxCore *core, PKey key) {
    return IsMouseButtonPressed(key);
}
bool GfxMouseDown(PanGfxCore *core, PKey key) { return IsMouseButtonDown(key); }
bool GfxMouseReleased(PanGfxCore *core, PKey key) {
    return IsMouseButtonReleased(key);
}
bool GfxMouseUp(PanGfxCore *core, PKey key) { return IsMouseButtonUp(key); }

void GfxGetMousePos(PanGfxCore *core, double *xpos, double *ypos) {
    Vector2 mpos = GetMousePosition();
    *xpos = (double)mpos.x;
    *ypos = (double)mpos.y;
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
    return CheckCollisionRecs(
        (Rectangle){x1, y1, w1, h1}, (Rectangle){x2, y2, w2, h2}
    );
}
bool GfxPointRectColsn(
    PanGfxCore *core, int px, int py, int rx, int ry, int rw, int rh
) {
    return CheckCollisionPointRec(
        (Vector2){px, py}, (Rectangle){rx, ry, rw, rh}
    );
}
