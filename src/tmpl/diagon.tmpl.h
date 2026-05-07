#ifndef PANKTI_DIAGON_H
#define PANKTI_DIAGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define NO_ERR_CTX_ERR "প্রয়োজনীয় প্রস্তুতি ছাড়াই পঙক্তি চালু হয়েছে"

typedef enum PanDiagCat{
	PAN_DIAG_LEXER = 0,
	PAN_DIAG_PARSER = 1,
	PAN_DIAG_COMPILER = 2,
	PAN_DIAG_RUNTIME = 3,
	PAN_DIAG_STRESCAPE = 4,
}PanDiagCat;

typedef enum PanDiagSeverity{
	PAN_DIAG_SEV_WARN = 0,
	PAN_DIAG_SEV_ERROR = 1,
}PanDiagSeverity;

typedef enum PanDiagCode{
//REPLACEME
	PANDIAG_CODE_COUNT
}PanDiagCode;

typedef struct PanDiagInfo{
	PanDiagCode code;
	PanDiagCat category;
	PanDiagSeverity severity;
	bool formatted;
	const char * msg;
	const char * hint;
}PanDiagInfo;

const PanDiagInfo * DiagGetInfo(PanDiagCode code);
const char * DiagGetMsg(PanDiagCode code);
const char * DiagGetHint(PanDiagCode code);


#ifdef __cplusplus
}
#endif

#endif
