#include "../include/alloc.h"
#include "../include/bengali.h"
#include "../include/ptypes.h"
#include "../include/ustring.h"
#include <ctype.h>
#include <math.h>

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
