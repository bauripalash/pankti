#include "../../include/gfxfont.h"
#include "../../external/kb/kb_text_shape.h"
#include "../../gen/fonts/notoserifbn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PanKbCtx *NewPanKbCtxFontContext(void) {
    PanKbCtx *c = malloc(sizeof(PanKbCtx));
    if (c == NULL) {
        return NULL;
    }

    c->fontSize = PANKB_DEFAULT_FONT_SIZE;
    c->fontFileData = NULL;
    c->fontFileSize = 0;
    c->kbCtx = NULL;
    return c;
}

void FreePanKbCtxFontContext(PanKbCtx *ctx) {
    if (ctx == NULL) {
        return;
    }
    if (ctx->kbCtx != NULL) {
        kbts_DestroyShapeContext(ctx->kbCtx);
    }

    if (ctx->fontFileData != NULL) {
        free(ctx->fontFileData);
    }
    free(ctx);
}
static unsigned char *brlkbReadFile(const char *path, size_t *size) {
    FILE *fptr = NULL;
    fptr = fopen(path, "rb");
    if (fptr == NULL) {
        return NULL;
    }
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    rewind(fptr);
    if (fsize == -1) {
        fclose(fptr);
        return NULL;
    }

    unsigned char *buffer = malloc(fsize);
    if (buffer == NULL) {
        fclose(fptr);
        return NULL;
    }

    size_t read = fread(buffer, 1, fsize, fptr);

    if (read != fsize) {
        fclose(fptr);
        free(buffer);
        return NULL;
    }

    fclose(fptr);
    *size = read;
    return buffer;
}

static bool brlkbSetup(PanKbCtx *ctx) {
    if (ctx == NULL) {
        return false;
    }
    if (ctx->fontFileData == NULL || ctx->fontFileSize == 0) {
        return false;
    }
    ctx->kbCtx = kbts_CreateShapeContext(0, 0);
    kbts_ShapePushFontFromMemory(
        ctx->kbCtx, ctx->fontFileData, ctx->fontFileSize, 0
    );
    stbtt_InitFont(
        &ctx->sFontInfo, ctx->fontFileData,
        stbtt_GetFontOffsetForIndex(ctx->fontFileData, 0)
    );
    return true;
}

static void brlkbResetSetup(PanKbCtx *ctx) { return; }

bool PanKbCtxLoadFontFile(PanKbCtx *ctx, const char *filepath) {
    if (ctx == NULL) {
        return false;
    }
    brlkbResetSetup(ctx);

    ctx->fontFileData = brlkbReadFile(filepath, &ctx->fontFileSize);
    if (ctx->fontFileData == NULL) {
        ctx->fontFileSize = 0;
        return false;
    }
    if (!brlkbSetup(ctx)) {
        return false;
    }
    return true;
}

bool PanKbCtxLoadFontFromMemory(
    PanKbCtx *ctx, unsigned char *data, size_t size
) {
    if (ctx == NULL) {
        return false;
    }
    brlkbResetSetup(ctx);
    ctx->fontFileData = data;
    ctx->fontFileSize = size;
    if (!brlkbSetup(ctx)) {
        return false;
    }
    return true;
}

bool PanKbLoadDefaultNotoFont(PanKbCtx *ctx) {
    return PanKbCtxLoadFontFromMemory(
        ctx, NOTOSERIFBENGALI_TTF, NOTOSERIFBENGALI_TTF_LEN
    );
}

static void brlkbDrawGlyph(
    PanKbCtx *ctx, int index, int posX, int posY, float scale, Color color
) {
    int width, height, xoffset, yoffset;
    unsigned char *bitmap = stbtt_GetGlyphBitmap(
        &ctx->sFontInfo, scale, scale, (int)index, &width, &height, &xoffset,
        &yoffset
    );
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char rawAlpha = bitmap[y * width + x];
            float alpha = rawAlpha / 255.0f;
            if (alpha == 0.0f) {
                continue;
            }

            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&ctx->sFontInfo, &ascent, &descent, &lineGap);
            int scaledAscent = (int)(ascent * scale);
            int pxPosX = posX + x + xoffset;
            int pxPosY = posY + y + scaledAscent + yoffset;
            DrawPixel(
                pxPosX, pxPosY,
                (Color){color.r, color.g, color.b, color.a * alpha}
            );
        }
    }
    stbtt_FreeBitmap(bitmap, NULL);
}

bool PanKbCtxDrawText(
    PanKbCtx *ctx, int xpos, int ypos, const char *text, Color color
) {
    kbts_ShapeBegin(ctx->kbCtx, KBTS_DIRECTION_LTR, KBTS_LANGUAGE_BANGLA);
    kbts_ShapeUtf8(
        ctx->kbCtx, text, strlen(text),
        KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX
    );
    kbts_ShapeEnd(ctx->kbCtx);
    float scale = stbtt_ScaleForPixelHeight(&ctx->sFontInfo, ctx->fontSize);

    float curX = xpos;
    float curY = ypos;
    // DrawLine(curX, curY, 500, curY, RED);
    // DrawLine(curX, curY, curX, 500, RED);

    // iterate through each glyph
    kbts_run Run;
    while (kbts_ShapeRun(ctx->kbCtx, &Run)) {
        kbts_glyph *Glyph;
        while (kbts_GlyphIteratorNext(&Run.Glyphs, &Glyph)) {
            int glyphIndex = Glyph->Id;

            float gxOffset = Glyph->OffsetX * scale;
            float gyOffset = Glyph->OffsetY * scale;
            float gxAdvance = Glyph->AdvanceX * scale;
            float gyAdvance = Glyph->AdvanceY * scale;

            brlkbDrawGlyph(
                ctx, glyphIndex, curX + gxOffset, curY + gyOffset, scale, color
            );
            curX += gxAdvance;
            curY += gyAdvance;
        }
    }

    return true;
}
