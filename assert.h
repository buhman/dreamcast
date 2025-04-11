#pragma once

#if defined(__dreamcast__)
#include "sh7091/serial.hpp"
#define print__character serial::character
#define print__string serial::string
#define print__integer serial::integer<uint32_t>
#else
#error "unknown platform"
#endif

#define assert(b)                                                       \
  do {                                                                  \
    if (!(b)) {                                                         \
      print__string(__FILE__);                                          \
      print__character(':');                                            \
      print__integer(__LINE__, ' ');                                    \
      print__string(__func__);                                          \
      print__string(": assertion failed: ");                            \
      print__string(#b);                                                \
      print__character('\n');                                           \
      while (1);                                                        \
    }                                                                   \
  } while (0);
