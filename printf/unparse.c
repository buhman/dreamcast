#include <stdint.h>

#include "minmax.h"
#include "unparse.h"

int digits_base10(uint32_t n)
{
  if (n >= 1000000000ul) return 10;
  if (n >= 100000000ul) return 9;
  if (n >= 10000000ul) return 8;
  if (n >= 1000000ul) return 7;
  if (n >= 100000ul) return 6;
  if (n >= 10000ul) return 5;
  if (n >= 1000ul) return 4;
  if (n >= 100ul) return 3;
  if (n >= 10ul) return 2;
  return 1;
}

int unparse_base10_unsigned(char * s, uint32_t n, int len, char fill)
{
  int digits = 0;
  digits += digits_base10(n);
  len = max(digits, len);
  int ret = len;

  while (len > digits) {
    *s++ = fill;
    --len;
  }

  while (len > 0) {
    const uint32_t digit = n % 10;
    n = n / 10;
    s[--len] = digit + 48;
  }

  return ret;
}

int unparse_base10(char * s, int32_t n, int len, char fill)
{
  bool negative = false;
  int digits = 0;
  if (n < 0) {
    digits += 1;
    n = -n;
    negative = true;
  }

  digits += digits_base10(n);
  len = max(digits, len);
  int ret = len;

  while (len > digits) {
    *s++ = fill;
    --len;
  }

  if (negative) {
    *s++ = '-';
    len--;
  }

  while (len > 0) {
    const uint32_t digit = n % 10;
    n = n / 10;
    s[--len] = digit + 48;
  }

  return ret;
}

int digits_base10_64(uint64_t n)
{
  if (n >= 10000000000000000000ull) return 20;
  if (n >= 1000000000000000000ull) return 19;
  if (n >= 100000000000000000ull) return 18;
  if (n >= 10000000000000000ull) return 17;
  if (n >= 1000000000000000ull) return 16;
  if (n >= 100000000000000ull) return 15;
  if (n >= 10000000000000ull) return 14;
  if (n >= 1000000000000ull) return 13;
  if (n >= 100000000000ull) return 12;
  if (n >= 10000000000ull) return 11;
  if (n >= 1000000000ull) return 10;
  if (n >= 100000000ull) return 9;
  if (n >= 10000000ull) return 8;
  if (n >= 1000000ull) return 7;
  if (n >= 100000ull) return 6;
  if (n >= 10000ull) return 5;
  if (n >= 1000ull) return 4;
  if (n >= 100ull) return 3;
  if (n >= 10ull) return 2;
  return 1;
}

int unparse_base10_64(char * s, int64_t n, int len, char fill)
{
  bool negative = false;
  int digits = 0;
  if (n < 0) {
    digits += 1;
    n = -n;
    negative = true;
  }

  digits += digits_base10_64(n);
  len = max(digits, len);
  int ret = len;

  while (len > digits) {
    *s++ = fill;
    --len;
  }

  if (negative) {
    *s++ = '-';
    len--;
  }

  while (len > 0) {
    const uint32_t digit = n % 10;
    n = n / 10;
    s[--len] = digit + 48;
  }

  return ret;
}

static int digits_base16(uint32_t n)
{
  if (n <= 0xf) return 1;
  if (n <= 0xff) return 2;
  if (n <= 0xfff) return 3;
  if (n <= 0xffff) return 4;
  if (n <= 0xfffff) return 5;
  if (n <= 0xffffff) return 6;
  if (n <= 0xfffffff) return 7;
  return 8;
}

int unparse_base16(char * s, uint32_t n, int len, char fill)
{
  int digits = digits_base16(n);
  len = max(digits, len);
  int ret = len;

  while (len > digits) {
    *s++ = fill;
    --len;
  }

  while (len > 0) {
    uint32_t nib = n & 0xf;
    n = n >> 4;
    if (nib > 9) {
      nib += (97 - 10);
    } else {
      nib += (48 - 0);
    }

    s[--len] = nib;
  }

  return ret;
}

#ifdef UNPARSE_TEST
#include <stdio.h>

int main()
{
  char s[1024];

  {
    int n = 124;

    int offset = unparse_base10(s, n, 6, ' ');
    s[offset] = 0;

    printf("`%s`\n", s);
  }

  {
    int n = 0x5678;

    int offset = unparse_base16(s, n, 7, '0');
    s[offset] = 0;

    printf("`%s`\n", s);
  }
}
#endif
