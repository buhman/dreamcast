#pragma once

#include <stdint.h>
#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

int unparse_base10_unsigned(char * s, uint32_t n, int len, char fill);
int unparse_base10(char * s, int32_t n, int len, char fill);
int unparse_base10_64(char * s, int64_t n, int len, char fill);
int unparse_base16(char * s, uint32_t n, int len, char fill);

int digits_base10(uint32_t n);
int digits_base_64(uint64_t n);
int digits_base10_64(uint64_t n);

#ifdef __cplusplus
}
#endif
