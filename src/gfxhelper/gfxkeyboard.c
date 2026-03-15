#include "../external/tigr/tigr.h"
#include "../include/gfxhelper.h"
#include "../include/ptypes.h"
#include "../include/utils.h"
#include <ctype.h>
#include <stdbool.h>

#define m(n) (StrEqual(keyStr, n))
int PanStrToMouseKey(const char *keyStr, i64 len) {
    (void)len;
    if (m(GFX_KEY_LEFT_1) || m(GFX_KEY_LEFT_2) || m(GFX_KEY_LEFT_3)) {
        return 1;
    } else if (m(GFX_KEY_RIGHT_1) || m(GFX_KEY_RIGHT_2) || m(GFX_KEY_RIGHT_3)) {
        return 2;

    } else if (m(GFX_MOUSE_KEY_MIDDLE_1) || m(GFX_MOUSE_KEY_MIDDLE_2) ||
               m(GFX_MOUSE_KEY_MIDDLE_3) || m(GFX_MOUSE_KEY_MIDDLE_3) ||
               m(GFX_MOUSE_KEY_MIDDLE_4) || m(GFX_MOUSE_KEY_MIDDLE_5)) {
        return 4;
    }

    return -1;
}

static PKey asciiCharToKeyboardKey(char keyChar) {
    if (isalpha(keyChar)) {
        // Key A to key Z
        // KEY_A -> KEY_Z
        return (PKey)toupper(keyChar);
    } else if (isdigit(keyChar)) {
        // Key 0 to Key 9
        // KEY_ZERO to KEY_NINE
        return (PKey)keyChar;
    } else {
        switch (keyChar) {
            case '\'': return TK_TICK;
            case ',': return TK_COMMA;
            case '-': return TK_MINUS;
            case '.': return TK_DOT;
            case '/': return TK_SLASH;
            case ';': return TK_SEMICOLON;
            case '=': return TK_EQUALS;
            case '[': return TK_LSQUARE;
            case '\\': return TK_BACKSLASH;
            case ']': return TK_RSQUARE;
            case '`': return TK_BACKTICK;
            case ' ': return TK_SPACE;
            default: return 0;
        }
    }
    return 0;
}

static PKey charKeyToKeyboardKey(char keyChar) {
    if (isascii(keyChar)) {
        return asciiCharToKeyboardKey(keyChar);
    }
    return 0;
}

static PKey fKeyToKeyboardKey(const char *numStr, u64 len) {

    bool ok = false;
    double nm = NumberFromStr(numStr, len, &ok);
    if (!ok) {
        return 0;
    }

    if (IsDoubleInt(nm) && nm > 0 && nm <= 12) {
        int num = (int)nm;
        return TK_PADDIV + num;
    }
    return 0;
}

PKey strMatchKeyName(const char *keyStr, i64 len) {

    if (m(GFX_KEY_ESCAPE_1) || m(GFX_KEY_ESCAPE_2) || m(GFX_KEY_ESCAPE_3)) {
        return TK_ESCAPE;
    } else if (m(GFX_KEY_ENTER_1) || m(GFX_KEY_ENTER_2) || m(GFX_KEY_ENTER_3)) {
        return TK_RETURN;
    } else if (m(GFX_KEY_TAB_1) || m(GFX_KEY_TAB_2) || m(GFX_KEY_TAB_3)) {
        return TK_TAB;
    } else if (m(GFX_KEY_BACKSPACE_1) || m(GFX_KEY_BACKSPACE_2) ||
               m(GFX_KEY_BACKSPACE_3)) {
        return TK_BACKSPACE;
    } else if (m(GFX_KEY_INSERT_1) || m(GFX_KEY_INSERT_2)) {
        return TK_INSERT;
    } else if (m(GFX_KEY_DELETE_1) || m(GFX_KEY_DELETE_2)) {
        return TK_DELETE;
    } else if (m(GFX_KEY_RIGHT_1) || m(GFX_KEY_RIGHT_2) || m(GFX_KEY_RIGHT_3)) {
        return TK_RIGHT;
    } else if (m(GFX_KEY_LEFT_1) || m(GFX_KEY_LEFT_2) || m(GFX_KEY_LEFT_3)) {
        return TK_LEFT;
    } else if (m(GFX_KEY_DOWN_1) || m(GFX_KEY_DOWN_2) || m(GFX_KEY_DOWN_3) ||
               m(GFX_KEY_DOWN_4) || m(GFX_KEY_DOWN_5)) {
        return TK_DOWN;
    } else if (m(GFX_KEY_UP_1) || m(GFX_KEY_UP_2) || m(GFX_KEY_UP_3) ||
               m(GFX_KEY_UP_4) || m(GFX_KEY_UP_5)) {
        return TK_UP;
    } else if (m(GFX_KEY_PAGEUP_1) || m(GFX_KEY_PAGEUP_2) ||
               m(GFX_KEY_PAGEUP_3) || m(GFX_KEY_PAGEUP_4) ||
               m(GFX_KEY_PAGEUP_5)) {
        return TK_PAGEUP;
    } else if (m(GFX_KEY_PAGEDOWN_1) || m(GFX_KEY_PAGEDOWN_2) ||
               m(GFX_KEY_PAGEDOWN_3) || m(GFX_KEY_PAGEDOWN_4) ||
               m(GFX_KEY_PAGEDOWN_5)) {
        return TK_PAGEDN;
    } else if (m(GFX_KEY_HOME_1) || m(GFX_KEY_HOME_2)) {
        return TK_HOME;
    } else if (m(GFX_KEY_END_1) || m(GFX_KEY_END_2)) {
        return TK_END;
    } else if (m(GFX_KEY_CAPSLOCK_1) || m(GFX_KEY_CAPSLOCK_2)) {
        return TK_CAPSLOCK;
    } else if (m(GFX_KEY_SCROLLLOCK_1) || m(GFX_KEY_SCROLLLOCK_2)) {
        return TK_SCROLL;
    } else if (m(GFX_KEY_NUMLOCKS_1) || m(GFX_KEY_NUMLOCKS_2)) {
        return TK_NUMLOCK;
    } else if (m(GFX_KEY_PRINTSCRN_1) || m(GFX_KEY_PRINTSCRN_2)) {
        return 0; // KEY_PRINT_SCREEN; TODO:
    } else if (m(GFX_KEY_PAUSE_1)) {
        return TK_PAUSE;
    } else if (m(GFX_KEY_LEFT_SHIFT_1) || m(GFX_KEY_LEFT_SHIFT_2) ||
               m(GFX_KEY_LEFT_SHIFT_3) || m(GFX_KEY_LEFT_SHIFT_4)) {
        return TK_LSHIFT;
    } else if (m(GFX_KEY_LEFT_CTRL_1) || m(GFX_KEY_LEFT_CTRL_2) ||
               m(GFX_KEY_LEFT_CTRL_3) || m(GFX_KEY_LEFT_CTRL_4) ||
               m(GFX_KEY_LEFT_CTRL_5)) {
        return TK_LCONTROL;
    } else if (m(GFX_KEY_LEFT_ALT_1) || m(GFX_KEY_LEFT_ALT_2) ||
               m(GFX_KEY_LEFT_ALT_3) || m(GFX_KEY_LEFT_ALT_4)) {
        return TK_LALT;
    } else if (m(GFX_KEY_LEFT_SUPER_1) || m(GFX_KEY_LEFT_SUPER_2) ||
               m(GFX_KEY_LEFT_SUPER_3) || m(GFX_KEY_LEFT_SUPER_4) ||
               m(GFX_KEY_LEFT_SUPER_5) || m(GFX_KEY_LEFT_SUPER_6) ||
               m(GFX_KEY_LEFT_SUPER_7) || m(GFX_KEY_LEFT_SUPER_8)) {
        return TK_LWIN;
    } else if (m(GFX_KEY_RIGHT_SHIFT_1) || m(GFX_KEY_RIGHT_SHIFT_2) ||
               m(GFX_KEY_RIGHT_SHIFT_3) || m(GFX_KEY_RIGHT_SHIFT_4)) {
        return TK_RSHIFT;
    } else if (m(GFX_KEY_RIGHT_CTRL_1) || m(GFX_KEY_RIGHT_CTRL_2) ||
               m(GFX_KEY_RIGHT_CTRL_3) || m(GFX_KEY_RIGHT_CTRL_4) ||
               m(GFX_KEY_RIGHT_CTRL_5)) {
        return TK_RCONTROL;
    } else if (m(GFX_KEY_RIGHT_ALT_1) || m(GFX_KEY_RIGHT_ALT_2) ||
               m(GFX_KEY_RIGHT_ALT_3) || m(GFX_KEY_RIGHT_ALT_4)) {
        return TK_RALT;
    } else if (m(GFX_KEY_RIGHT_SUPER_1) || m(GFX_KEY_RIGHT_SUPER_2) ||
               m(GFX_KEY_RIGHT_SUPER_3) || m(GFX_KEY_RIGHT_SUPER_4) ||
               m(GFX_KEY_RIGHT_SUPER_5) || m(GFX_KEY_RIGHT_SUPER_6) ||
               m(GFX_KEY_RIGHT_SUPER_7) || m(GFX_KEY_RIGHT_SUPER_8)) {
        return TK_RWIN;
    } else if (m(GFX_KEY_MENU_1) || m(GFX_KEY_MENU_2)) {
        return 0;
    } else if (m(GFX_KEY_KP_ZERO_1) || m(GFX_KEY_KP_ZERO_2) ||
               m(GFX_KEY_KP_ZERO_3)) {
        return TK_PAD0;
    } else if (m(GFX_KEY_KP_ONE_1) || m(GFX_KEY_KP_ONE_2) ||
               m(GFX_KEY_KP_ONE_3)) {
        return TK_PAD1;
    } else if (m(GFX_KEY_KP_TWO_1) || m(GFX_KEY_KP_TWO_2) ||
               m(GFX_KEY_KP_TWO_3)) {
        return TK_PAD2;
    } else if (m(GFX_KEY_KP_THREE_1) || m(GFX_KEY_KP_THREE_2) ||
               m(GFX_KEY_KP_THREE_3)) {
        return TK_PAD3;
    } else if (m(GFX_KEY_KP_FOUR_1) || m(GFX_KEY_KP_FOUR_2) ||
               m(GFX_KEY_KP_FOUR_3)) {
        return TK_PAD4;
    } else if (m(GFX_KEY_KP_FIVE_1) || m(GFX_KEY_KP_FIVE_2) ||
               m(GFX_KEY_KP_FIVE_3)) {
        return TK_PAD5;
    } else if (m(GFX_KEY_KP_SIX_1) || m(GFX_KEY_KP_SIX_2) ||
               m(GFX_KEY_KP_SIX_3)) {
        return TK_PAD6;
    } else if (m(GFX_KEY_KP_SEVEN_1) || m(GFX_KEY_KP_SEVEN_2) ||
               m(GFX_KEY_KP_SEVEN_3)) {
        return TK_PAD7;
    } else if (m(GFX_KEY_KP_EIGHT_1) || m(GFX_KEY_KP_EIGHT_2) ||
               m(GFX_KEY_KP_EIGHT_3)) {
        return TK_PAD8;
    } else if (m(GFX_KEY_KP_NINE_1) || m(GFX_KEY_KP_NINE_2) ||
               m(GFX_KEY_KP_NINE_3)) {
        return TK_PAD9;
    } else if (m(GFX_KEY_KP_DECIMAL_1) || m(GFX_KEY_KP_DECIMAL_2) ||
               m(GFX_KEY_KP_DECIMAL_3)) {
        return TK_PADDOT;
    } else if (m(GFX_KEY_KP_DIVIDE_1) || m(GFX_KEY_KP_DIVIDE_2) ||
               m(GFX_KEY_KP_DIVIDE_3)) {
        return TK_PADDIV;
    } else if (m(GFX_KEY_KP_MULTIPLY_1) || m(GFX_KEY_KP_MULTIPLY_2) ||
               m(GFX_KEY_KP_MULTIPLY_3)) {
        return TK_PADMUL;
    } else if (m(GFX_KEY_KP_SUBTRACT_1) || m(GFX_KEY_KP_SUBTRACT_2) ||
               m(GFX_KEY_KP_SUBTRACT_3)) {
        return TK_PADSUB;
    } else if (m(GFX_KEY_KP_ADD_1) || m(GFX_KEY_KP_ADD_2) ||
               m(GFX_KEY_KP_ADD_3)) {
        return TK_PADADD;
    } else if (m(GFX_KEY_KP_ENTER_1) || m(GFX_KEY_KP_ENTER_2) ||
               m(GFX_KEY_KP_ENTER_3)) {
        return TK_PADENTER;
    } else if (m(GFX_KEY_KP_EQUAL_1) || m(GFX_KEY_KP_EQUAL_2) ||
               m(GFX_KEY_KP_EQUAL_3)) {
        return TK_PADENTER;
    }
    return 0;
}

PKey PanStrToKeyboardKey(const char *keyStr, i64 len) {
    u64 keyLen = 0;

    if (len != -1) {
        keyLen = (u64)len;
    } else {
        keyLen = StrLength(keyStr);
    }

    if (keyLen == 1) {
        char keyChar = keyStr[0];
        return charKeyToKeyboardKey(keyChar);
    }

    if (keyStr[0] == 'f' || keyStr[0] == 'F') {
        const char *ptr = ++keyStr;
        return fKeyToKeyboardKey(ptr, keyLen - 1);
    }

    return strMatchKeyName(keyStr, len);
}
