#include "../include/alloc.h"
#include "../include/bengali.h"
#include "../include/ptypes.h"
#include "../include/ustring.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

bool FormatDouble(double num, char *buf, int bufsize) {
    // from prec 1 to 17 (possible max)
    // we convert it to string then convert it to double again and match
    // it with `num`. if it matches means we got the shortest length
    // otherwise (which doesn't seem to possible in practice) we return the
    // whole thing converted to string
    //
    // Here how it works
    // target=1.2345
    // prec=1 -> 1 -> match -> false -> continue
    // prec=2 -> 1.2 -> match -> false -> continue
    // prec=3 -> 1.23 -> match -> false -> continue
    // ...
    // prec=5 -> 1.2345 -> match -> true -> return
    for (int prec = 1; prec <= 17; prec++) {
        snprintf(buf, (size_t)bufsize, "%.*g", prec, num);
        if (strtod(buf, NULL) == num) {
            return true;
        }
    }
    snprintf(buf, (size_t)bufsize, "%.17g", num);
    return true;
}

void NumberStrToBnStr(const char *en, char *buf, int bufSize) {
    // write index
    size_t wi = 0;
    for (size_t ri = 0; en[ri] != '\0'; ri++) {
        char ch = en[ri];
        if (ch >= '0' && ch <= '9') {
            if (wi + 3 >= bufSize) break;
            buf[wi++] = (char)0xE0;
            buf[wi++] = (char)0xA7;
            buf[wi++] = (char)(0xA6 + (ch - '0'));
        } else {
            if (wi + 1 >= bufSize) break;
            buf[wi++] = (char)ch;
        }
    }
    buf[wi] = '\0';
}

double NumberFromStr(const char *lexeme, u64 len, bool *ok) {
    char *buf = PCalloc(len + 1, sizeof(char));
    if (buf == NULL) {
        *ok = false;
        return -1;
    }

    UIter *iter = NewUIterator(lexeme);
    int index = 0;

    while (!UIterIsEnd(iter)) {
        char32 ch = UIterNext(iter);
        if (ch == '.') {
            buf[index++] = '.';
            continue;
        }
        buf[index++] = GetEnFromBnNum(ch);
    }

    double value = atof(buf);
    *ok = true;
    FreeUIterator(iter);
    PFree(buf);
    return value;
}

double ClampDouble(double value, double min, double max) {
    if (value <= min) {
        return min;
    }
    if (value >= max) {
        return max;
    }

    return value;
}

i64 ClampInt(i64 value, i64 min, i64 max) {
    if (value <= min) {
        return min;
    }
    if (value >= max) {
        return max;
    }

    return value;
}

bool IsDoubleSafeInt(double d) {
    return d >= (double)INT_MIN && d <= (double)INT_MAX;
}
bool IsU64SafeInt(u64 u) { return u <= (u64)INT_MAX; }
bool IsI64SafeInt(i64 i) { return i >= (i64)INT_MIN && i <= (i64)INT_MAX; }

bool IsDoubleInt(double d) { return (floor(d) == ceil(d)); }

unsigned char ToHex2Bytes(char c1, char c2) {
    unsigned char nb1, nb2;

    if (isalpha(c1)) {
        nb1 = (unsigned char)(tolower(c1) - 'a') + 10;
    } else if (isdigit(c1)) {
        nb1 = (unsigned char)(c1 - '0');
    } else {
        return 0;
    }

    if (isalpha(c2)) {
        nb2 = (unsigned char)(tolower(c2) - 'a') + 10;
    } else if (isdigit(c2)) {
        nb2 = (unsigned char)(c2 - '0');
    } else {
        return 0;
    }

    return (unsigned char)(nb1 << 4) | nb2;
}

unsigned char HexStrToByte(char *str, int len) {
    if (len == 2) {
        return ToHex2Bytes(str[0], str[1]);
    }

    return 0;
}
