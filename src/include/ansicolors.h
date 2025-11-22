#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TERM_ESC
#define TERM_ESC "\x1b"
#endif

#define TERMC_RESET   TERM_ESC "[0m"
#define TERMC_BLACK   TERM_ESC "[0;30m"
#define TERMC_RED     TERM_ESC "[0;31m"
#define TERMC_GREEN   TERM_ESC "[0;32m"
#define TERMC_YELLOW  TERM_ESC "[0;33m"
#define TERMC_BLUE    TERM_ESC "[0;34m"
#define TERMC_PURPLE  TERM_ESC "[0;35m"
#define TERMC_CYAN    TERM_ESC "[0;36m"
#define TERMC_WHITE   TERM_ESC "[0;37m"

#define TERMC_BBLACK  TERM_ESC "[0;30m"
#define TERMC_BRED    TERM_ESC "[0;31m"
#define TERMC_BGREEN  TERM_ESC "[0;32m"
#define TERMC_BYELLOW TERM_ESC "[0;33m"
#define TERMC_BBLUE   TERM_ESC "[0;34m"
#define TERMC_BPURPLE TERM_ESC "[0;35m"
#define TERMC_BCYAN   TERM_ESC "[0;36m"
#define TERMC_BWHITE  TERM_ESC "[0;37m"

#define TERMC_UBLACK  TERM_ESC "[4;30m"
#define TERMC_URED    TERM_ESC "[4;31m"
#define TERMC_UGREEN  TERM_ESC "[4;32m"
#define TERMC_UYELLOW TERM_ESC "[4;33m"
#define TERMC_UBLUE   TERM_ESC "[4;34m"
#define TERMC_UPURPLE TERM_ESC "[4;35m"
#define TERMC_UCYAN   TERM_ESC "[4;36m"
#define TERMC_UWHITE  TERM_ESC "[4;37m"

#ifdef __cplusplus
}
#endif

#endif
