#ifndef PANKTI_BRLKB_H
#define PANKTI_BRLKB_H

#include "../external/raylib/raylib.h"
#include <stdbool.h>
#include <stddef.h>

#include "../external/kb/kb_text_shape.h"
#include "../external/stb/stb_truetype.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PANKB_DEFAULT_FONT_SIZE 28

// Raylib+kb_text_shape Integration Context
typedef struct PanKbCtx {
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

// Create a new Raylib+kb_text_shape integration context Font Context
PanKbCtx *NewPanKbCtxFontContext(void);
// Free Raylib+kb_text_shape Font Context
void FreePanKbCtxFontContext(PanKbCtx *ctx);
// Load a font file. Resets previous states and buffer text
bool PanKbCtxLoadFontFile(PanKbCtx *ctx, const char *filepath);
// Draw text with previously loaded font from context
bool PanKbCtxDrawText(
    PanKbCtx *ctx, int xpos, int ypos, const char *text, Color color
);
// Load default font (Noto Serif Bengali Regular)
bool PanKbLoadDefaultNotoFont(PanKbCtx *ctx);

#ifdef __cplusplus
}
#endif

#endif
