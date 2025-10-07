#ifndef UTILS_H
#define UTILS_H
#include <uchar.h>
#include <stdbool.h>

#define ArrCount(arr)     (sizeof(arr) / sizeof(arr[0]))

//Read file to string (must free the returned string)
char * ReadFile(const char * path);

//Create a substring from `str` which is str[start...end];
//(must free the returned string)
char * SubString(const char * str, int start, int end);

bool StrEqual(const char * str1, const char * str2);
long StrLength(const char * str);

bool MatchKW(const char * s, const char * en, const char * pn, const char * bn);

char32_t U8ToU32(const unsigned char * str);

#endif
