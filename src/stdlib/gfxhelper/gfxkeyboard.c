#include "../../external/raylib/raylib.h"
#include "../../include/gfxhelper.h"
#include "../../include/ptypes.h"
#include "../../include/utils.h"
#include <ctype.h>
#include <stdbool.h>

static KeyboardKey asciiCharToKeyboardKey(char keyChar) {
    if (isalpha(keyChar)) {
        // Key A to key Z
        // KEY_A -> KEY_Z
        return (KeyboardKey)toupper(keyChar);
    } else if (isdigit(keyChar)) {
        // Key 0 to Key 9
        // KEY_ZERO to KEY_NINE
        return (KeyboardKey)keyChar;
    } else {
        switch (keyChar) {
            case '\'': return KEY_APOSTROPHE;
            case ',': return KEY_COMMA;
            case '-': return KEY_MINUS;
            case '.': return KEY_PERIOD;
            case '/': return KEY_SLASH;
            case ';': return KEY_SEMICOLON;
            case '=': return KEY_EQUAL;
            case '[': return KEY_LEFT_BRACKET;
            case '\\': return KEY_BACKSLASH;
            case ']': return KEY_RIGHT_BRACKET;
            case '`': return KEY_GRAVE;
            case ' ': return KEY_SPACE;
            default: return KEY_NULL;
        }
    }
    return KEY_NULL;
}

static KeyboardKey charKeyToKeyboardKey(char keyChar) {
    if (isascii(keyChar)) {
        return asciiCharToKeyboardKey(keyChar);
    }
    return KEY_NULL;
}

static KeyboardKey fKeyToKeyboardKey(const char *numStr, u64 len) {

    bool ok = false;
    double nm = NumberFromStr(numStr, len, &ok);
    if (!ok) {
        return KEY_NULL;
    }

    if (IsDoubleInt(nm) && nm > 0 && nm <= 12) {
        int num = (int)nm;
        return 289 + num;
    }
    return KEY_NULL;
}

static KeyboardKey kbKeyToKeyboardKey(const char *keyStr, u64 len) {
    return KEY_NULL;
}

#define m(n) (StrEqual(keyStr, n))

KeyboardKey strMatchKeyName(const char *keyStr, i64 len) {

    if (m(GFX_KEY_ESCAPE_1) || m(GFX_KEY_ESCAPE_2) || m(GFX_KEY_ESCAPE_3)) {
        return KEY_ESCAPE;
    } else if (m(GFX_KEY_ENTER_1) || m(GFX_KEY_ENTER_2) || m(GFX_KEY_ENTER_3)) {
        return KEY_ENTER;
    } else if (m(GFX_KEY_TAB_1) || m(GFX_KEY_TAB_2) || m(GFX_KEY_TAB_3)) {
        return KEY_TAB;
    } else if (m(GFX_KEY_BACKSPACE_1) || m(GFX_KEY_BACKSPACE_2) ||
               m(GFX_KEY_BACKSPACE_3)) {
        return KEY_BACKSPACE;
    } else if (m(GFX_KEY_INSERT_1) || m(GFX_KEY_INSERT_2)) {
        return KEY_INSERT;
    } else if (m(GFX_KEY_DELETE_1) || m(GFX_KEY_DELETE_2)) {
        return KEY_DELETE;
    } else if (m(GFX_KEY_RIGHT_1) || m(GFX_KEY_RIGHT_2) || m(GFX_KEY_RIGHT_3)) {
        return KEY_RIGHT;
    } else if (m(GFX_KEY_LEFT_1) || m(GFX_KEY_LEFT_2) || m(GFX_KEY_LEFT_3)) {
        return KEY_LEFT;
    } else if (m(GFX_KEY_DOWN_1) || m(GFX_KEY_DOWN_2) || m(GFX_KEY_DOWN_3) ||
               m(GFX_KEY_DOWN_4) || m(GFX_KEY_DOWN_5)) {
        return KEY_DOWN;
    } else if (m(GFX_KEY_UP_1) || m(GFX_KEY_UP_2) || m(GFX_KEY_UP_3) ||
               m(GFX_KEY_UP_4) || m(GFX_KEY_UP_5)) {
        return KEY_UP;
    } else if (m(GFX_KEY_PAGEUP_1) || m(GFX_KEY_PAGEUP_2) ||
               m(GFX_KEY_PAGEUP_3) || m(GFX_KEY_PAGEUP_4) ||
               m(GFX_KEY_PAGEUP_5)) {
        return KEY_PAGE_UP;
    } else if (m(GFX_KEY_PAGEDOWN_1) || m(GFX_KEY_PAGEDOWN_2) ||
               m(GFX_KEY_PAGEDOWN_3) || m(GFX_KEY_PAGEDOWN_4) ||
               m(GFX_KEY_PAGEDOWN_5)) {
        return KEY_PAGE_DOWN;
    } else if (m(GFX_KEY_HOME_1) || m(GFX_KEY_HOME_2)) {
        return KEY_HOME;
    } else if (m(GFX_KEY_END_1) || m(GFX_KEY_END_2)) {
        return KEY_END;
    } else if (m(GFX_KEY_CAPSLOCK_1) || m(GFX_KEY_CAPSLOCK_2)) {
        return KEY_CAPS_LOCK;
    } else if (m(GFX_KEY_SCROLLLOCK_1) || m(GFX_KEY_SCROLLLOCK_2)) {
        return KEY_SCROLL_LOCK;
    } else if (m(GFX_KEY_NUMLOCKS_1) || m(GFX_KEY_NUMLOCKS_2)) {
        return KEY_NUM_LOCK;
    } else if (m(GFX_KEY_PRINTSCRN_1) || m(GFX_KEY_PRINTSCRN_2)) {
        return KEY_PRINT_SCREEN;
    } else if (m(GFX_KEY_PAUSE_1)) {
        return KEY_PAUSE;
    } else if (m(GFX_KEY_LEFT_SHIFT_1) || m(GFX_KEY_LEFT_SHIFT_2) ||
               m(GFX_KEY_LEFT_SHIFT_3) || m(GFX_KEY_LEFT_SHIFT_4)) {
        return KEY_LEFT_SHIFT;
    } else if (m(GFX_KEY_LEFT_CTRL_1) || m(GFX_KEY_LEFT_CTRL_2) ||
               m(GFX_KEY_LEFT_CTRL_3) || m(GFX_KEY_LEFT_CTRL_4) ||
               m(GFX_KEY_LEFT_CTRL_5)) {
        return KEY_LEFT_CONTROL;
    } else if (m(GFX_KEY_LEFT_ALT_1) || m(GFX_KEY_LEFT_ALT_2) ||
               m(GFX_KEY_LEFT_ALT_3) || m(GFX_KEY_LEFT_ALT_4)) {
        return KEY_LEFT_ALT;
    } else if (m(GFX_KEY_LEFT_SUPER_1) || m(GFX_KEY_LEFT_SUPER_2) ||
               m(GFX_KEY_LEFT_SUPER_3) || m(GFX_KEY_LEFT_SUPER_4) ||
               m(GFX_KEY_LEFT_SUPER_5) || m(GFX_KEY_LEFT_SUPER_6) ||
               m(GFX_KEY_LEFT_SUPER_7) || m(GFX_KEY_LEFT_SUPER_8)) {
        return KEY_LEFT_SUPER;
    } else if (m(GFX_KEY_RIGHT_SHIFT_1) || m(GFX_KEY_RIGHT_SHIFT_2) ||
               m(GFX_KEY_RIGHT_SHIFT_3) || m(GFX_KEY_RIGHT_SHIFT_4)) {
        return KEY_RIGHT_SHIFT;
    } else if (m(GFX_KEY_RIGHT_CTRL_1) || m(GFX_KEY_RIGHT_CTRL_2) ||
               m(GFX_KEY_RIGHT_CTRL_3) || m(GFX_KEY_RIGHT_CTRL_4) ||
               m(GFX_KEY_RIGHT_CTRL_5)) {
        return KEY_RIGHT_CONTROL;
    } else if (m(GFX_KEY_RIGHT_ALT_1) || m(GFX_KEY_RIGHT_ALT_2) ||
               m(GFX_KEY_RIGHT_ALT_3) || m(GFX_KEY_RIGHT_ALT_4)) {
        return KEY_RIGHT_ALT;
    } else if (m(GFX_KEY_RIGHT_SUPER_1) || m(GFX_KEY_RIGHT_SUPER_2) ||
               m(GFX_KEY_RIGHT_SUPER_3) || m(GFX_KEY_RIGHT_SUPER_4) ||
               m(GFX_KEY_RIGHT_SUPER_5) || m(GFX_KEY_RIGHT_SUPER_6) ||
               m(GFX_KEY_RIGHT_SUPER_7) || m(GFX_KEY_RIGHT_SUPER_8)) {
        return KEY_RIGHT_SUPER;
    } else if (m(GFX_KEY_MENU_1) || m(GFX_KEY_MENU_2)) {
        return KEY_KB_MENU;
    }
    return KEY_NULL;
}

KeyboardKey PanStrToKeyboardKey(const char *keyStr, i64 len) {
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

    if (keyStr[0] == 'k' || keyStr[0] == 'K') {
        const char *ptr = ++keyStr;
    }

    return strMatchKeyName(keyStr, len);
}
