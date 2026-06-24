/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "alloc.h"
#include "external/stb/stb_image.h"
#include "gen/panktilogo.h"
#include "gfx.h"
#include "printer.h"

#if defined(PANKTI_OS_LINUX)
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static void gfxSetX11AppClass(PanGfxCore *core) {
    Display *dpy = NULL;
    Window win = {0};
    PCTigrExposeX11Handle(core->screen, &dpy, &win);
    if (dpy == NULL) {
        return;
    }

    XClassHint *classHint = XAllocClassHint();
    if (classHint != NULL) {
        classHint->res_name = "pankti"; // TODO: take from core or options
        classHint->res_class =
            "in.palashbauri.pankti"; // TODO: take from core or options
        XSetClassHint(dpy, win, classHint);
        XFree(classHint);
    }
}

static void gfxSetX11Icon(PanGfxCore *core) {
    int w, h, channels;
    unsigned char *img = stbi_load_from_memory(
        IMAGES_PANKTI_LOGO_PNG, IMAGES_PANKTI_LOGO_PNG_LEN, &w, &h, &channels, 4
    );
    if (img == NULL) {
        return;
    }

    long *icon = PMalloc(sizeof(long) * (w * h + 2));
    if (icon == NULL) {
        PanPrint("[WARN] Failed to set GUI icon");
        return;
    }
    icon[0] = w;
    icon[1] = h;

    for (int i = 0; i < w * h; ++i) {
        unsigned char r = img[i * 4 + 0];
        unsigned char g = img[i * 4 + 1];
        unsigned char b = img[i * 4 + 2];
        unsigned char a = img[i * 4 + 3];
        icon[i + 2] = ((unsigned long)a << 24) | ((unsigned long)r << 16) |
                      ((unsigned long)g << 8) | (unsigned long)b;
    }
    Display *dpy = NULL;
    Window win = {0};
    PCTigrExposeX11Handle(core->screen, &dpy, &win);
    if (dpy == NULL) {
        return;
    }

    Atom prop = XInternAtom(dpy, "_NET_WM_ICON", False);
    XChangeProperty(
        dpy, win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)icon,
        w * h + 2
    );
    XFlush(dpy);
    PFree(icon);
    stbi_image_free(img);
}
#endif

#if defined(PANKTI_OS_WIN)
#include <windows.h>

static void gfxSetWin32Icon(PanGfxCore *core) {
    int w, h, channels;
    unsigned char *img = stbi_load_from_memory(
        IMAGES_PANKTI_LOGO_PNG, IMAGES_PANKTI_LOGO_PNG_LEN, &w, &h, &channels, 4
    );
    if (img == NULL) {
        return;
    }

    unsigned char *bgraIcon = PMalloc(w * h * 4);
    if (bgraIcon == NULL) {
        PanPrint("[WARN] Failed to set GUI icon");
        return;
    }
    for (int i = 0; i < w * h; ++i) {
        bgraIcon[i * 4 + 0] = img[i * 4 + 2];
        bgraIcon[i * 4 + 1] = img[i * 4 + 1];
        bgraIcon[i * 4 + 2] = img[i * 4 + 0];
        bgraIcon[i * 4 + 3] = img[i * 4 + 3];
    }

    ICONINFO iinfo = {0};
    iinfo.fIcon = TRUE;
    iinfo.xHotspot = 0;
    iinfo.yHotspot = 0;

    HDC hdc = GetDC(NULL);
    HBITMAP color = CreateBitmap(w, h, 1, 32, bgraIcon);
    HBITMAP mask = CreateBitmap(w, h, 1, 32, NULL);

    iinfo.hbmColor = color;
    iinfo.hbmMask = mask;
    HICON icon = CreateIconIndirect(&iinfo);
    HWND hwnd = (HWND)core->screen->handle;
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);

    DeleteObject(mask);
    DeleteObject(color);
    ReleaseDC(NULL, hdc);
    PFree(bgraIcon);
    stbi_image_free(img);
}

#endif

void GfxSetWindowIcon(PanGfxCore *core) {
#if defined(PANKTI_OS_LINUX)
    gfxSetX11AppClass(core);
    gfxSetX11Icon(core);
#elif defined(PANKTI_OS_WIN)
    gfxSetWin32Icon(core);
#else
    return;
#endif
}
