#pragma once

#if defined(__dreamcast__)
#include "sh7091/c_serial.h"
#else
#include <stdio.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void print_char(char c)
{
#if defined(__dreamcast__)
  sh7091_character(c);
#else
  fputc(c, stderr);
#endif
}

void print_string(const char * s, int length);
void print_bytes(const uint8_t * s, int length);
void print_chars(const uint16_t * s, int length);
void print_cstring(const char * s);
void print_integer(const int n);

void _printf(const char * format, ...);

#define printf(...) _printf(__VA_ARGS__)
#define printc(c) print_char(c)
#define prints(s) print_cstring(s)

#if defined(DEBUG_PRINT)
#define debugf(...) _printf(__VA_ARGS__)
#define debugc(c) print_char(c)
#define debugs(s) print_cstring(s)
#else
#define debugf(...)
#define debugc(c)
#define debugs(c)
#endif

#ifdef __cplusplus
}
#endif
