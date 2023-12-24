#pragma once

#include <stdint.h>

#ifndef USE_SH2_DVSR
inline constexpr uint32_t
__udiv32(uint32_t n, uint32_t d)
{
  uint32_t q = 0;
  uint32_t r = 0;

  for (int i = 31; i >= 0; --i) {
    q = q << 1;
    r = r << 1;

    r |= (n >> 31) & 1;
    n = n << 1;

    if (d <= r) {
      r = r - d;
      q = q | 1;
    }
  }

  return q;
}

inline constexpr uint32_t
__udiv64_32(uint64_t n, uint32_t base)
{
  uint64_t rem = n;
  uint64_t b = base;
  uint64_t res = 0, d = 1;
  uint32_t high = rem >> 32;

  if (high >= base) {
    high = __udiv32(high, base);
    res = (uint64_t)high << 32;
    rem -= (uint64_t)(high*base) << 32;
  }

  while ((int64_t)b > 0 && b < rem) {
    b = b+b;
    d = d+d;
  }

  do {
    if (rem >= b) {
      rem -= b;
      res += d;
    }
    b >>= 1;
    d >>= 1;
  } while (d);

  return res;
}
#else
#include "sh2.h"
inline uint32_t
__udiv64_32(uint64_t n, uint32_t d)
{
  sh2.reg.DVSR = d;
  sh2.reg.DVDNTH = (uint32_t)(n >> 32);
  sh2.reg.DVDNTL = (uint32_t)(n);

  // 39 cycles
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");

  return sh2.reg.DVDNTL;
}
#endif

inline int32_t
__div64_32(int64_t n, int32_t d)
{
  uint64_t n_abs = n >= 0 ? (uint64_t)n : -(uint64_t)n;
  uint32_t d_abs = d >= 0 ? (uint32_t)d : -(uint32_t)d;
  uint32_t q_abs = __udiv64_32(n_abs, d_abs);

  return (n < 0) == (d < 0) ? (int32_t)q_abs : -(int32_t)q_abs;
}
