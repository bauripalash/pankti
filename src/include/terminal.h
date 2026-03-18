#ifndef PANKTI_TERMINAL_H
#define PANKTI_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

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

typedef struct PTermInfo {
    bool color;
    bool underline;
} PTermInfo;

PTermInfo GetTermInfo(void);
void InitTermInfo(void);

const char *TermColor(const char *color);

const char *TermReset(void);

const char *TermBlack(void);
const char *TermWhite(void);

const char *TermRed(void);
const char *TermGreen(void);
const char *TermBlue(void);
const char *TermCyan(void);

const char *TermYellow(void);
const char *TermPurple(void);

#ifdef __cplusplus
}
#endif

#endif
