#ifndef PANKTI_RL_HELPER_H
#define PANKTI_RL_HELPER_H

#include "ptypes.h"
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

#define GFX_KEY_ESCAPE_1       "ইস্কেপ"
#define GFX_KEY_ESCAPE_2       "esc"
#define GFX_KEY_ESCAPE_3       "escape"

#define GFX_KEY_ENTER_1        "এন্টার"
#define GFX_KEY_ENTER_2        "enter"
#define GFX_KEY_ENTER_3        "enter"

#define GFX_KEY_TAB_1          "ট্যাব"
#define GFX_KEY_TAB_2          "tab"
#define GFX_KEY_TAB_3          "tab"

#define GFX_KEY_BACKSPACE_1    "ব্যাকস্পেস"
#define GFX_KEY_BACKSPACE_2    "backspace"
#define GFX_KEY_BACKSPACE_3    "backspace"

#define GFX_KEY_INSERT_1       "ইনসার্ট"
#define GFX_KEY_INSERT_2       "insert"

#define GFX_KEY_DELETE_1       "ডিলিট"
#define GFX_KEY_DELETE_2       "delete"

#define GFX_KEY_RIGHT_1        "ডান"
#define GFX_KEY_RIGHT_2        "right"
#define GFX_KEY_RIGHT_3        "dan"

#define GFX_KEY_LEFT_1         "বাম"
#define GFX_KEY_LEFT_2         "left"
#define GFX_KEY_LEFT_3         "bam"

#define GFX_KEY_DOWN_1         "নীচে"
#define GFX_KEY_DOWN_2         "down"
#define GFX_KEY_DOWN_3         "niche"
#define GFX_KEY_DOWN_4         "নীচ"
#define GFX_KEY_DOWN_5         "nich"

#define GFX_KEY_UP_1           "উপরে"
#define GFX_KEY_UP_2           "up"
#define GFX_KEY_UP_3           "upore"
#define GFX_KEY_UP_4           "উপর"
#define GFX_KEY_UP_5           "upor"

#define GFX_KEY_PAGEUP_1       "পেজ উপর"
#define GFX_KEY_PAGEUP_2       "page up"
#define GFX_KEY_PAGEUP_3       "page upor"
#define GFX_KEY_PAGEUP_4       "পেজ উপরে"
#define GFX_KEY_PAGEUP_5       "page upore"

#define GFX_KEY_PAGEDOWN_1     "পেজ নীচ" // TODO: Check spelling
#define GFX_KEY_PAGEDOWN_2     "page down"
#define GFX_KEY_PAGEDOWN_3     "page nich"
#define GFX_KEY_PAGEDOWN_4     "পেজ নীচে"
#define GFX_KEY_PAGEDOWN_5     "page niche"

#define GFX_KEY_HOME_1         "হোম"
#define GFX_KEY_HOME_2         "home"

#define GFX_KEY_END_1          "এন্ড"
#define GFX_KEY_END_2          "end"

#define GFX_KEY_CAPSLOCK_1     "ক্যাপস লক"
#define GFX_KEY_CAPSLOCK_2     "caps lock"

#define GFX_KEY_SCROLLLOCK_1   "স্ক্রোল লক"
#define GFX_KEY_SCROLLLOCK_2   "scroll lock"

#define GFX_KEY_NUMLOCKS_1     "নাম লক"
#define GFX_KEY_NUMLOCKS_2     "num lock"

#define GFX_KEY_PRINTSCRN_1    "প্রিন্ট স্ক্রিন"
#define GFX_KEY_PRINTSCRN_2    "print screen"

#define GFX_KEY_PAUSE_1        "pause"

#define GFX_KEY_LEFT_SHIFT_1   "লেফট শিফট"
#define GFX_KEY_LEFT_SHIFT_2   "left shift"
#define GFX_KEY_LEFT_SHIFT_3   "বাম শিফট"
#define GFX_KEY_LEFT_SHIFT_4   "bam shift"

#define GFX_KEY_LEFT_CTRL_1    "লেফট কন্ট্রোল"
#define GFX_KEY_LEFT_CTRL_2    "left control"
#define GFX_KEY_LEFT_CTRL_3    "left ctrl"
#define GFX_KEY_LEFT_CTRL_4    "বাম কন্ট্রোল"
#define GFX_KEY_LEFT_CTRL_5    "bam control"

#define GFX_KEY_LEFT_ALT_1     "লেফট অল্ট"
#define GFX_KEY_LEFT_ALT_2     "left alt"
#define GFX_KEY_LEFT_ALT_3     "বাম অল্ট"
#define GFX_KEY_LEFT_ALT_4     "bam alt"

#define GFX_KEY_LEFT_SUPER_1   "লেফট সুপার"
#define GFX_KEY_LEFT_SUPER_2   "left super"
#define GFX_KEY_LEFT_SUPER_3   "লেফট উইন্ডোজ"
#define GFX_KEY_LEFT_SUPER_4   "left windows"
#define GFX_KEY_LEFT_SUPER_5   "বাম সুপার"
#define GFX_KEY_LEFT_SUPER_6   "bam super"
#define GFX_KEY_LEFT_SUPER_7   "বাম উইন্ডোজ"
#define GFX_KEY_LEFT_SUPER_8   "bam windows"

#define GFX_KEY_RIGHT_SHIFT_1  "রাইট শিফট"
#define GFX_KEY_RIGHT_SHIFT_2  "right shift"
#define GFX_KEY_RIGHT_SHIFT_3  "ডান শিফট"
#define GFX_KEY_RIGHT_SHIFT_4  "dan shift"

#define GFX_KEY_RIGHT_CTRL_1   "রাইট কন্ট্রোল"
#define GFX_KEY_RIGHT_CTRL_2   "right control"
#define GFX_KEY_RIGHT_CTRL_3   "right ctrl"
#define GFX_KEY_RIGHT_CTRL_4   "ডান কন্ট্রোল"
#define GFX_KEY_RIGHT_CTRL_5   "dan control"

#define GFX_KEY_RIGHT_ALT_1    "রাইট অল্ট"
#define GFX_KEY_RIGHT_ALT_2    "right alt"
#define GFX_KEY_RIGHT_ALT_3    "ডান অল্ট"
#define GFX_KEY_RIGHT_ALT_4    "dan alt"

#define GFX_KEY_RIGHT_SUPER_1  "রাইট সুপার"
#define GFX_KEY_RIGHT_SUPER_2  "right super"
#define GFX_KEY_RIGHT_SUPER_3  "রাইট উইন্ডোজ"
#define GFX_KEY_RIGHT_SUPER_4  "right windows"
#define GFX_KEY_RIGHT_SUPER_5  "ডান সুপার"
#define GFX_KEY_RIGHT_SUPER_6  "dan super"
#define GFX_KEY_RIGHT_SUPER_7  "ডান উইন্ডোজ"
#define GFX_KEY_RIGHT_SUPER_8  "dan windows"

#define GFX_KEY_MENU_1         "menu"
#define GFX_KEY_MENU_2         "মেনু"

#define GFX_KEY_KP_ZERO_1      "কীপ্যাড ০"
#define GFX_KEY_KP_ZERO_2      "keypad 0"
#define GFX_KEY_KP_ZERO_3      "kp 0"

#define GFX_KEY_KP_ONE_1       "কীপ্যাড ১"
#define GFX_KEY_KP_ONE_2       "keypad 0"
#define GFX_KEY_KP_ONE_3       "kp1"

#define GFX_KEY_KP_TWO_1       "কীপ্যাড ২"
#define GFX_KEY_KP_TWO_2       "keypad 2"
#define GFX_KEY_KP_TWO_3       "kp2"

#define GFX_KEY_KP_THREE_1     "কীপ্যাড ৩"
#define GFX_KEY_KP_THREE_2     "keypad 3"
#define GFX_KEY_KP_THREE_3     "kp3"

#define GFX_KEY_KP_FOUR_1      "কীপ্যাড ৪"
#define GFX_KEY_KP_FOUR_2      "keypad 4"
#define GFX_KEY_KP_FOUR_3      "kp4"

#define GFX_KEY_KP_FIVE_1      "কীপ্যাড ৫"
#define GFX_KEY_KP_FIVE_2      "keypad 5"
#define GFX_KEY_KP_FIVE_3      "kp5"

#define GFX_KEY_KP_SIX_1       "কীপ্যাড ৬"
#define GFX_KEY_KP_SIX_2       "keypad 6"
#define GFX_KEY_KP_SIX_3       "kp6"

#define GFX_KEY_KP_SEVEN_1     "কীপ্যাড ৭"
#define GFX_KEY_KP_SEVEN_2     "keypad 7"
#define GFX_KEY_KP_SEVEN_3     "kp7"

#define GFX_KEY_KP_EIGHT_1     "কীপ্যাড ৮"
#define GFX_KEY_KP_EIGHT_2     "keypad 8"
#define GFX_KEY_KP_EIGHT_3     "kp8"

#define GFX_KEY_KP_NINE_1      "কীপ্যাড ৯"
#define GFX_KEY_KP_NINE_2      "keypad 9"
#define GFX_KEY_KP_NINE_3      "kp9"

#define GFX_KEY_KP_DECIMAL_1   "কীপ্যাড ."
#define GFX_KEY_KP_DECIMAL_2   "keypad ."
#define GFX_KEY_KP_DECIMAL_3   "kp."

#define GFX_KEY_KP_DIVIDE_1    "কীপ্যাড /"
#define GFX_KEY_KP_DIVIDE_2    "keypad /"
#define GFX_KEY_KP_DIVIDE_3    "kp/"

#define GFX_KEY_KP_MULTIPLY_1  "কীপ্যাড *"
#define GFX_KEY_KP_MULTIPLY_2  "keypad *"
#define GFX_KEY_KP_MULTIPLY_3  "kp*"

#define GFX_KEY_KP_SUBTRACT_1  "কীপ্যাড -"
#define GFX_KEY_KP_SUBTRACT_2  "keypad -"
#define GFX_KEY_KP_SUBTRACT_3  "kp-"

#define GFX_KEY_KP_ADD_1       "কীপ্যাড +"
#define GFX_KEY_KP_ADD_2       "keypad +"
#define GFX_KEY_KP_ADD_3       "kp+"

#define GFX_KEY_KP_ENTER_1     "কীপ্যাড এন্টার"
#define GFX_KEY_KP_ENTER_2     "keypad enter"
#define GFX_KEY_KP_ENTER_3     "kp enter"

#define GFX_KEY_KP_EQUAL_1     "কীপ্যাড ="
#define GFX_KEY_KP_EQUAL_2     "keypad ="
#define GFX_KEY_KP_EQUAL_3     "kp="

#define GFX_MOUSE_KEY_MIDDLE_1 "মাঝের"
#define GFX_MOUSE_KEY_MIDDLE_2 "মাঝ"
#define GFX_MOUSE_KEY_MIDDLE_3 "majher"
#define GFX_MOUSE_KEY_MIDDLE_4 "majh"
#define GFX_MOUSE_KEY_MIDDLE_5 "middle"

#define GFX_IMAGE_STR_PREFIX   "ছবি:"

Color ParseColorString(const char *str, ColorStrError *err);

Color PanStrToColor(const char *str, ColorStrError *err);
Image LoadGuiAppIcon(void);
KeyboardKey PanStrToKeyboardKey(const char *keyStr, i64 len);
int PanStrToMouseKey(const char *keyStr, i64 len);
#ifdef __cplusplus
}
#endif

#endif
