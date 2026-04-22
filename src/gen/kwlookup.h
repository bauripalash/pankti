/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf -L ANSI-C -C -D -t -l -N PanktiKeywordLookup  */
/* Computed positions: -k'1,3,6' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif


#include <string.h>
#include "../include/token.h"
struct PanktiKeyword{
	const char * name;
	PTokenType ttype;
};

#define TOTAL_KEYWORDS 51
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 18
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 74
/* maximum key range = 73, duplicates = 1 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75,  5, 10, 25,
       5,  0, 25, 75, 10, 15, 15, 25,  0, 25,
       0,  0, 10, 75,  5,  5, 15, 55, 75,  0,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75,  0, 20, 75, 75, 75, 75,
      75, 75, 75, 30, 75, 75, 75, 75, 75, 20,
      75, 75, 75, 75, 30, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 15, 75, 35, 30, 10, 75,
      75, 10, 20,  0,  0, 10, 10, 75, 75, 75,
      75, 75, 15, 75, 10, 75, 75, 75, 75, 75,
       5, 10, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 10, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 5:
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

const struct PanktiKeyword *
PanktiKeywordLookup (register const char *str, register size_t len)
{
  static const unsigned char lengthtable[] =
    {
       2,  3,  3,  5,  2,  3,  4,  5,  6,  2,  3,  4,  5,  6,
       2,  3,  4,  5,  6,  4,  5,  6, 12,  4,  5,  6, 12,  8,
       4,  5,  6, 18,  9, 15,  6,  3,  9, 15,  6,  8,  9, 15,
       6, 18,  9, 15,  9, 15,  9,  9,  4
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
  static const struct PanktiKeyword wordlist[] =
    {
      {"or", T_OR},
      {"nil", T_NIL},
      {"nil", T_NIL},
      {"ebong", T_AND},
      {"do", T_DO},
      {"end", T_END},
      {"else", T_ELSE},
      {"dhori", T_LET},
      {"anoyon", T_IMPORT},
      {"ba", T_OR},
      {"and", T_AND},
      {"sesh", T_END},
      {"break", T_BREAK},
      {"nahole", T_ELSE},
      {"if", T_IF},
      {"let", T_LET},
      {"then", T_THEN},
      {"while", T_WHILE},
      {"bhango", T_BREAK},
      {"jodi", T_IF},
      {"sotti", T_TRUE},
      {"return", T_RETURN},
      {"\340\246\255\340\246\276\340\246\231\340\247\213", T_BREAK},
      {"func", T_FUNC},
      {"false", T_FALSE},
      {"tahole", T_THEN},
      {"\340\246\206\340\246\250\340\247\237\340\246\250", T_IMPORT},
      {"continue", T_CONTINUE},
      {"koro", T_DO},
      {"ferao", T_RETURN},
      {"chalao", T_CONTINUE},
      {"\340\246\256\340\246\277\340\246\245\340\247\215\340\246\257\340\246\276", T_FALSE},
      {"\340\246\250\340\246\277\340\246\262", T_NIL},
      {"\340\246\250\340\246\276\340\246\271\340\246\262\340\247\207", T_ELSE},
      {"\340\246\254\340\246\276", T_OR},
      {"kaj", T_FUNC},
      {"\340\246\225\340\246\276\340\246\234", T_FUNC},
      {"\340\246\244\340\246\276\340\246\271\340\246\262\340\247\207", T_THEN},
      {"import", T_IMPORT},
      {"jotokhon", T_WHILE},
      {"\340\246\225\340\246\260\340\247\213", T_DO},
      {"\340\246\270\340\246\244\340\247\215\340\246\257\340\246\277", T_TRUE},
      {"mittha", T_FALSE},
      {"\340\246\257\340\246\244\340\246\225\340\247\215\340\246\267\340\246\243", T_WHILE},
      {"\340\246\266\340\247\207\340\246\267", T_END},
      {"\340\246\253\340\247\207\340\246\260\340\246\276\340\246\223", T_RETURN},
      {"\340\246\247\340\246\260\340\246\277", T_LET},
      {"\340\246\232\340\246\276\340\246\262\340\246\276\340\246\223", T_CONTINUE},
      {"\340\246\257\340\246\246\340\246\277", T_IF},
      {"\340\246\217\340\246\254\340\246\202", T_AND},
      {"true", T_TRUE}
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

  static const signed char lookup[] =
    {
       -1,  -1,   0, -74,  -1,   3,  -1,   4,   5,   6,
        7,   8,   9,  10,  11,  12,  13,  14,  15,  16,
       17,  18, -50,  -2,  19,  20,  21,  22,  -1,  23,
       24,  25,  26,  27,  28,  29,  30,  -1,  31,  32,
       33,  34,  -1,  35,  36,  37,  38,  -1,  39,  40,
       41,  42,  -1,  43,  44,  45,  -1,  -1,  -1,  46,
       47,  -1,  -1,  -1,  48,  -1,  -1,  -1,  -1,  49,
       -1,  -1,  -1,  -1,  50
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              if (len == lengthtable[index])
                {
                  register const char *s = wordlist[index].name;

                  if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                    return &wordlist[index];
                }
            }
          else if (index < -TOTAL_KEYWORDS)
            {
              register int offset = - 1 - TOTAL_KEYWORDS - index;
              register const unsigned char *lengthptr = &lengthtable[TOTAL_KEYWORDS + lookup[offset]];
              register const struct PanktiKeyword *wordptr = &wordlist[TOTAL_KEYWORDS + lookup[offset]];
              register const struct PanktiKeyword *wordendptr = wordptr + -lookup[offset + 1];

              while (wordptr < wordendptr)
                {
                  if (len == *lengthptr)
                    {
                      register const char *s = wordptr->name;

                      if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                        return wordptr;
                    }
                  lengthptr++;
                  wordptr++;
                }
            }
        }
    }
  return (struct PanktiKeyword *) 0;
}

