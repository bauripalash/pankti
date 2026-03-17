#include "include/terminal.h"
#include "include/system.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdlib.h>

#if defined(PANKTI_OS_WIN)
#include <io.h>
#include <windows.h>
#define PAN_ISATTY(fd) _isatty(fd)
#define PAN_STDERR_FD  2
#else
#include <unistd.h>
#define PAN_ISATTY(fd) isatty(fd)
#define PAN_STDERR_FD  STDERR_FILENO
#endif

static PTermInfo pantermInfo = {
    .color = false,
    .underline = false,
};

static bool checkNoColor(void) { return getenv("NO_COLOR") != NULL; }

static bool checkIsAtty(void) { return PAN_ISATTY(PAN_STDERR_FD) != 0; }

static bool checkTermColor(void) {
    const char *term = getenv("TERM");
    if (term == NULL) {
        return false;
    }

    bool result =
        (StrStartsWith(term, "color") || StrStartsWith(term, "256") ||
         StrStartsWith(term, "xterm") || StrStartsWith(term, "screen") ||
         StrStartsWith(term, "tmux") || StrStartsWith(term, "rxvt") ||
         StrStartsWith(term, "linux") || StrStartsWith(term, "vt100"));
    return result;
}

void InitTermInfo(void) {
    if (!checkIsAtty()) {
        return;
    }
    if (checkNoColor()) {
        return;
    }

    pantermInfo.color = checkTermColor();
    if (pantermInfo.color) {
        pantermInfo.underline = true;
    }
}

PTermInfo GetTermInfo(void) { return pantermInfo; }
const char *TermColor(const char *color) {
    if (pantermInfo.color) {
        return color;
    } else {
        return "";
    }
}
