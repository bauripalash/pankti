#ifndef UTILS_H
#define UTILS_H
#include <uchar.h>

//Read file to string (must free the returned string)
char * ReadFile(const char * path);

//Create a substring from `str` which is str[start...end];
//(must free the returned string)
char * SubString(const char * str, int start, int end);

char32_t U8ToU32(const unsigned char * str);
#endif
