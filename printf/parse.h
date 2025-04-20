#pragma once

#include <stdint.h>
#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * parse_skip(const char * s, char c);
const char * parse_find(const char * s, char c);
const char * parse_find_first_right(const char * s, int length, char c);
int parse_base10_digit(char c);
const char * parse_base10(const char * s, int * n);
const char * parse_base10_64(const char * s, int64_t * n);
const char * parse_match(const char * s, const char * m);
int parse_stride(const char * s, int length);
int parse_height(const char * s, int length);

#ifdef __cplusplus
}
#endif
