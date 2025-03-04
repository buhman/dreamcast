#include <stddef.h>

int parse_base10_digit(char c);
const char * parse_base10(const char * s, int * n);
const char * parse_base10_64(const char * s, int64_t * n);
