#ifndef PANKTI_TERMINAL_H
#define PANKTI_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

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
