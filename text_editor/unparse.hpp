#pragma once

#include <stdint.h>

int unparse_base10_unsigned(char * s, uint32_t n, int len, char fill);
int unparse_base10(char * s, int32_t n, int len, char fill);
int unparse_base10_64(char * s, int64_t n, int len, char fill);
int unparse_base16(char * s, uint32_t n, int len, char fill);
