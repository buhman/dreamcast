#pragma once

#include "printf/printf.h"

#define print__character print_char
#define print__string print_cstring
#define print__integer print_integer

#define assert(b)                                                       \
  do {                                                                  \
    if (!(b)) {                                                         \
      print__string(__FILE__);                                          \
      print__character(':');                                            \
      print__integer(__LINE__);                                         \
      print__character(' ');                                            \
      print__string(__func__);                                          \
      print__string(": assertion failed: ");                            \
      print__string(#b);                                                \
      print__character('\n');                                           \
      while (1);                                                        \
    }                                                                   \
  } while (0);
