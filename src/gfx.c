/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gfx.h"
#include "alloc.h"
#include "external/stb/stb_ds.h"
#include "ptypes.h"
#include <stdbool.h>
#include <string.h>

PanGfxCore *NewGfxCore(int width, int height, const char *title, int fontSize) {
    PanGfxCore *core = PCreate(PanGfxCore);
    if (core == NULL) {
        return NULL;
    }

    PanKbCtx *fctx = NewPanKbCtxFontContext();
    if (fctx == NULL) {
        PFree(core);
        return NULL;
    }

    core->fontCtx = fctx;
    core->screen = NULL;
    core->win = NULL;
    core->imageList = NULL;

    if (width >= 0) {
        core->winWidth = width;
    } else {
        core->winHeight = DEFAULT_STDGFX_WIN_WIDTH;
    }

    if (height >= 0) {
        core->winHeight = height;
    } else {
        core->winHeight = DEFAULT_STDGFX_WIN_HEIGHT;
    }

    if (title != NULL) {
        strcpy(core->winTitle, title);
    } else {
        strcpy(core->winTitle, DEFAULT_STDGFX_WIN_TITLE);
    }

    if (fontSize >= 0) {
        core->fontCtx->fontSize = fontSize;
    } else {
        core->fontCtx->fontSize = DEFAULT_STDGFX_FONT_SIZE;
    }

    core->winRunning = false;

    PanKbLoadDefaultNotoFont(core->fontCtx);
    core->screen = NULL;
    core->fontCtx->core = core;
    core->initd = true;
    core->mousePrevBtn = 0;
    core->mouseNewPress = 0;
    core->mousePrevBtn = 0;
    return core;
}

static void unloadImageList(PanGfxCore *core) {
    if (core == NULL || core->imageList == NULL) {
        return;
    }
    i64 count = arrlen(core->imageList);
    for (i64 i = count; i > 0; i--) {
        tigrFree(arrpop(core->imageList));
    }

    arrfree(core->imageList);
    core->imageList = NULL;
}

void FreeGfxCore(PanGfxCore *core) {
    if (core == NULL) {
        return;
    }

    unloadImageList(core);

    if (core->fontCtx != NULL) {
        FreePanKbCtxFontContext(core->fontCtx);
    }

    core->initd = false;
}

bool StartGfxProcess(PanGfxCore *core) {
    if (core == NULL) {
        return false;
    }
    if (!core->initd) {
        return false;
    }

    core->winRunning = true;
    core->screen =
        tigrWindow(core->winWidth, core->winHeight, core->winTitle, TIGR_AUTO);
    GfxSetWindowIcon(core);
    return true;
}

bool EndGfxProcess(PanGfxCore *core) {
    if (core->screen != NULL) {
        tigrFree(core->screen);
    }
    return true;
}

bool UpdateGfxStatus(PanGfxCore *core) {
    core->winRunning = !tigrClosed(core->screen);
    int x = 0;
    int y = 0;
    int buttons = 0;
    tigrMouse(core->screen, &x, &y, &buttons);
    core->mouseNewPress = buttons & ~core->mousePrevBtn;
    core->mouseNewRelease = ~buttons & core->mousePrevBtn;
    core->mousePrevBtn = buttons;

    for (int key = 0; key < GFX_CORE_MAX_KEYS; ++key) {
        int down = tigrKeyDown(core->screen, key);
        core->kbPressedKey[key] = down && !core->kbPrevKeys[key];
        core->kbReleasedKey[key] = !down && core->kbPrevKeys[key];
        core->kbPrevKeys[key] = down;
    }
    return true;
}
