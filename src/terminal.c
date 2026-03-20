#include "include/terminal.h"
#include "include/system.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdlib.h>

#ifndef TERM_ESC
#define TERM_ESC "\x1b"
#endif

#define TERMC_RESET     TERM_ESC "[0m"
#define TERMC_BLACK     TERM_ESC "[0;30m"
#define TERMC_RED       TERM_ESC "[0;31m"
#define TERMC_GREEN     TERM_ESC "[0;32m"
#define TERMC_YELLOW    TERM_ESC "[0;33m"
#define TERMC_BLUE      TERM_ESC "[0;34m"
#define TERMC_PURPLE    TERM_ESC "[0;35m"
#define TERMC_CYAN      TERM_ESC "[0;36m"
#define TERMC_WHITE     TERM_ESC "[0;37m"

#define TERMC_UNDERLINE TERM_ESC "[4m"
#define TERMC_BOLD      TERM_ESC "[1m"

#if defined(PANKTI_OS_WIN)
#include <io.h>
#include <windows.h>
#define PAN_ISATTY(fd) _isatty(fd)
#define PAN_STDERR_FD  2
#define PAN_STDOUT_FD  1
#else
#include <unistd.h>
#define PAN_ISATTY(fd) isatty(fd)
#define PAN_STDERR_FD  STDERR_FILENO
#define PAN_STDOUT_FD  STDOUT_FILENO
#endif

static PTermInfo pantermInfo = {
    .color = false,
    .underline = false,
};

static bool checkNoColor(void) { return getenv("NO_COLOR") != NULL; }

static bool checkIsAtty(void) {
    return (PAN_ISATTY(PAN_STDERR_FD) != 0 && PAN_ISATTY(PAN_STDOUT_FD) != 0);
}

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

#if defined(PANKTI_OS_WIN)

    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);

    if (h != INVALID_HANDLE_VALUE) {

        DWORD mode = 0;

        if (GetConsoleMode(h, &mode)) {

            if (SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                pantermInfo.color = true;
                pantermInfo.underline = true;
                return;
            }
        }
    }

    return;

#endif

    pantermInfo.color = checkTermColor();
    if (pantermInfo.color) {
        pantermInfo.underline = true;
    }
}

PTermInfo GetTermInfo(void) { return pantermInfo; }

const char *TermColor(const char *color) {
    return pantermInfo.color ? color : "";
}

const char *TermReset(void) { return pantermInfo.color ? TERMC_RESET : ""; }

const char *TermBlack(void) { return pantermInfo.color ? TERMC_BLACK : ""; }
const char *TermWhite(void) { return pantermInfo.color ? TERMC_WHITE : ""; }
const char *TermRed(void) { return pantermInfo.color ? TERMC_RED : ""; }
const char *TermGreen(void) { return pantermInfo.color ? TERMC_GREEN : ""; }

const char *TermBlue(void) { return pantermInfo.color ? TERMC_BLUE : ""; }

const char *TermCyan(void) { return pantermInfo.color ? TERMC_CYAN : ""; }

const char *TermYellow(void) { return pantermInfo.color ? TERMC_YELLOW : ""; }

const char *TermPurple(void) { return pantermInfo.color ? TERMC_PURPLE : ""; }
const char *TermUnderline(void) {
    return pantermInfo.underline ? TERMC_UNDERLINE : "";
}
const char *TermBold(void) { return pantermInfo.color ? TERMC_BOLD : ""; }

const char *TermErrorColorStart(void) {
    if (pantermInfo.color) {
        return TERMC_RED TERMC_UNDERLINE TERMC_BOLD;
    }

    return " -->";
}
const char *TermErrorColorEnd(void) {
    if (pantermInfo.color) {
        return TERMC_RESET;
    }

    return "<--";
}
