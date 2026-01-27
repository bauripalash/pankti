#ifndef PANKTI_RL_HELPER_H
#define PANKTI_RL_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../external/raylib/raylib.h"
#include <stdbool.h>

// Color sources : https://en.wikipedia.org/wiki/Web_colors

// prefix for custom-color string
#define GFX_COLOR_PREFIX_BN    "রঙ="
#define GFX_COLOR_PREFIX_EN    "color="
#define GFX_COLOR_PREFIX_EN2   "colour="

#define GFX_COLOR_GRAY_CODE    (Color){255, 255, 255, 255}
#define GFX_COLOR_GRAY_BN      "ধুসর"
#define GFX_COLOR_GRAY_EN      "gray"

#define GFX_COLOR_YELLOW_CODE  (Color){255, 255, 0, 255}
#define GFX_COLOR_YELLOW_BN    "হলুদ"
#define GFX_COLOR_YELLOW_EN    "yellow"

#define GFX_COLOR_GOLD_CODE    (Color){255, 215, 0, 255}
#define GFX_COLOR_GOLD_BN      "সোনালি"
#define GFX_COLOR_GOLD_EN      "gold"

#define GFX_COLOR_ORANGE_CODE  (Color){255, 165, 0, 255}
#define GFX_COLOR_ORANGE_BN    "কমলা"
#define GFX_COLOR_ORANGE_EN    "orange"

#define GFX_COLOR_PINK_CODE    (Color){255, 192, 203, 255}
#define GFX_COLOR_PINK_BN      "গোলাপি"
#define GFX_COLOR_PINK_EN      "pink"

#define GFX_COLOR_RED_CODE     (Color){255, 0, 0, 255}
#define GFX_COLOR_RED_BN       "লাল"
#define GFX_COLOR_RED_EN       "red"

#define GFX_COLOR_GREEN_CODE   (Color){0, 255, 0, 255}
#define GFX_COLOR_GREEN_BN     "সবুজ"
#define GFX_COLOR_GREEN_EN     "green"

#define GFX_COLOR_SKYBLUE_CODE (Color){135, 206, 235, 255}
#define GFX_COLOR_SKYBLUE_BN   "আকাশি"
#define GFX_COLOR_SKYBLUE_EN   "sky blue"

#define GFX_COLOR_BLUE_CODE    (Color){0, 0, 255, 255}
#define GFX_COLOR_BLUE_BN      "নীল"
#define GFX_COLOR_BLUE_EN      "blue"

#define GFX_COLOR_PURPLE_CODE  (Color){128, 0, 128, 255}
#define GFX_COLOR_PURPLE_BN    "বেগুনি"
#define GFX_COLOR_PURPLE_EN    "purple"

#define GFX_COLOR_VIOLET_CODE  (Color){238, 130, 238, 255}
#define GFX_COLOR_VIOLET_BN    "হালকা বেগুনি"
#define GFX_COLOR_VIOLET_EN    "violet"

#define GFX_COLOR_BROWN_CODE   (Color){165, 42, 42, 255}
#define GFX_COLOR_BROWN_BN     "বাদামি"
#define GFX_COLOR_BROWN_EN     "brown"

#define GFX_COLOR_WHITE_CODE   (Color){255, 255, 255, 255}
#define GFX_COLOR_WHITE_BN     "সাদা"
#define GFX_COLOR_WHITE_EN     "white"

#define GFX_COLOR_BLACK_CODE   (Color){0, 0, 0, 255}
#define GFX_COLOR_BLACK_BN     "কাল"
#define GFX_COLOR_BLACK_EN     "black"

#define GFX_COLOR_SAFFRON_CODE (Color){255, 119, 34, 255}; // Indian saffron
#define GFX_COLOR_SAFFRON_BN   "গেরুয়া"
#define GFX_COLOR_SAFFRON_EN   "saffron"

#define GFX_COLOR_BLANK_CODE   (Color){0, 0, 0, 0}

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

Color ParseColorString(const char *str, ColorStrError *err);

Color PanStrToColor(const char *str, ColorStrError *err);
void LoadGuiAppIcon(void);
#ifdef __cplusplus
}
#endif

#endif
