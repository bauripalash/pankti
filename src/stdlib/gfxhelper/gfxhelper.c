#include "../../include/gfxhelper.h"
#include "../../external/raylib/raylib.h"
#include "../../gen/panktilogo.h"
#include "../../include/utils.h"
#include "raylib.h"
#include <math.h>
#include <stdbool.h>

#define MatchClr(clr)         (StrEqual(str, clr))
#define MatchClr2(clr1, clr2) (StrEqual(str, clr1) || StrEqual(str, clr2))

Color ParseColorString(const char *str, ColorStrError *err) {
    const char *ptr = str;
    if (StrStartsWith(str, GFX_COLOR_PREFIX_EN)) {
        ptr += StrLength(GFX_COLOR_PREFIX_EN);
    } else if (StrStartsWith(str, GFX_COLOR_PREFIX_BN)) {
        ptr += StrLength(GFX_COLOR_PREFIX_BN);
    } else if (StrStartsWith(str, GFX_COLOR_PREFIX_EN2)) {
        ptr += StrLength(GFX_COLOR_PREFIX_EN2);
    } else {
        *err = CLRSTR_PREFIX_NOT_FOUND;
        return GFX_COLOR_BLANK_CODE;
    }

    int clrCodeCount = 0;
    char **clrValues = StrSplit(ptr, ',', &clrCodeCount);
    if (clrCodeCount < 3) {
        *err = CLRSTR_TOO_LESS_VALS;
        return GFX_COLOR_BLANK_CODE;
    }

    if (clrCodeCount > 4) {
        *err = CLRSTR_TOO_MUCH_VALS;
        return GFX_COLOR_BLANK_CODE;
    }

    int rVal = 0;
    int gVal = 0;
    int bVal = 0;
    int aVal = 255;

    bool numParseOk = false;
    double rawRed =
        NumberFromStr(clrValues[0], StrLength(clrValues[0]), &numParseOk);
    if (!numParseOk) {
        *err = CLRSTR_INVALID_COLOR_VAL_R;
        return GFX_COLOR_BLANK_CODE;
    }
    numParseOk = false;

    double rawGreen =
        NumberFromStr(clrValues[1], StrLength(clrValues[1]), &numParseOk);
    if (!numParseOk) {

        *err = CLRSTR_INVALID_COLOR_VAL_G;
        return GFX_COLOR_BLANK_CODE;
    }
    numParseOk = false;

    double rawBlue =
        NumberFromStr(clrValues[2], StrLength(clrValues[2]), &numParseOk);
    if (!numParseOk) {

        *err = CLRSTR_INVALID_COLOR_VAL_B;
        return GFX_COLOR_BLANK_CODE;
    }
    numParseOk = false;
    double rawAlpha = 255.0f;
    if (clrCodeCount == 4) {
        rawAlpha =
            NumberFromStr(clrValues[3], StrLength(clrValues[3]), &numParseOk);
        if (!numParseOk) {
            *err = CLRSTR_INVALID_COLOR_VAL_A;
            return GFX_COLOR_BLANK_CODE;
        }
    }

    rVal = round(ClampDouble(rawRed, 0, 255));
    gVal = round(ClampDouble(rawGreen, 0, 255));
    bVal = round(ClampDouble(rawBlue, 0, 255));
    aVal = round(ClampDouble(rawAlpha, 0, 255));
    *err = CLRSTR_OK;
    return (Color){rVal, gVal, bVal, aVal};
}

Color PanStrToColor(const char *str, ColorStrError *err) {
    ColorStrError parseErr = CLRSTR_OK;
    Color val = ParseColorString(str, &parseErr);
    if (parseErr == CLRSTR_OK) {
        *err = parseErr;
        return val;
    }
    if (parseErr != CLRSTR_PREFIX_NOT_FOUND) {
        *err = parseErr;
        return val;
    }
    if (MatchClr2(GFX_COLOR_GRAY_BN, GFX_COLOR_GRAY_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_GRAY_CODE;
    } else if (MatchClr2(GFX_COLOR_YELLOW_BN, GFX_COLOR_YELLOW_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_YELLOW_CODE;
    } else if (MatchClr2(GFX_COLOR_GOLD_BN, GFX_COLOR_GOLD_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_GOLD_CODE;
    } else if (MatchClr2(GFX_COLOR_ORANGE_BN, GFX_COLOR_ORANGE_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_ORANGE_CODE;
    } else if (MatchClr2(GFX_COLOR_PINK_BN, GFX_COLOR_PINK_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_PINK_CODE;
    } else if (MatchClr2(GFX_COLOR_RED_BN, GFX_COLOR_RED_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_RED_CODE;
    } else if (MatchClr2(GFX_COLOR_GREEN_BN, GFX_COLOR_GREEN_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_GREEN_CODE;
    } else if (MatchClr2(GFX_COLOR_SKYBLUE_BN, GFX_COLOR_SKYBLUE_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_SKYBLUE_CODE;
    } else if (MatchClr2(GFX_COLOR_BLUE_BN, GFX_COLOR_BLUE_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_BLUE_CODE;
    } else if (MatchClr2(GFX_COLOR_VIOLET_BN, GFX_COLOR_VIOLET_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_VIOLET_CODE;
    } else if (MatchClr2(GFX_COLOR_PURPLE_BN, GFX_COLOR_PURPLE_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_PURPLE_CODE;
    } else if (MatchClr2(GFX_COLOR_BROWN_BN, GFX_COLOR_BROWN_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_BROWN_CODE;
    } else if (MatchClr2(GFX_COLOR_WHITE_BN, GFX_COLOR_WHITE_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_WHITE_CODE;
    } else if (MatchClr2(GFX_COLOR_BLACK_BN, GFX_COLOR_BLACK_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_BLACK_CODE;
    } else if (MatchClr2(GFX_COLOR_SAFFRON_BN, GFX_COLOR_SAFFRON_EN)) {
        *err = CLRSTR_OK;
        return GFX_COLOR_SAFFRON_CODE; // Indian saffron
    }

    *err = CLRSTR_UNKNOWN_CLR_NAME;
    return GFX_COLOR_BLANK_CODE;
}

void LoadGuiAppIcon(void) {
    Image iconImg = LoadImageFromMemory(
        ".png", IMAGES_PANKTI_LOGO_PNG, IMAGES_PANKTI_LOGO_PNG_LEN
    );
    SetWindowIcon(iconImg);
}
