#include <stddef.h>

#include "parse.h"

int parse_base10_digit(char c)
{
  switch (c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  default: return -1;
  }
}

const char * parse_base10(const char * s, int * n)
{
  *n = 0;
  int sign = 1;

  if (*s == '-') {
    sign = -1;
    s++;
  }

  while (true) {
    int digit = parse_base10_digit(*s);
    if (digit == -1)
      break;

    *n *= 10;
    *n += digit;
    s++;
  }

  *n *= sign;

  return s;
}

const char * parse_base10_64(const char * s, int64_t * n)
{
  *n = 0;
  int sign = 1;

  if (*s == '-') {
    sign = -1;
    s++;
  }

  while (true) {
    int digit = parse_base10_digit(*s);
    if (digit == -1)
      break;

    *n *= 10;
    *n += digit;
    s++;
  }

  *n *= sign;

  return s;
}
