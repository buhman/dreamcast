#pragma once

#include <cstdint>

namespace string {

  template <typename T>
  inline void hex(T * c, uint32_t len, uint32_t n)
  {
    while (len > 0) {
      uint32_t nib = n & 0xf;
      n = n >> 4;
      if (nib > 9) {
	nib += (97 - 10);
      } else {
	nib += (48 - 0);
      }

      c[--len] = nib;
    }
  }

  template <typename T>
  inline void dec(T * c, uint32_t len, uint32_t n)
  {
    while (len > 0) {
      const uint32_t digit = n % 10;
      n = n / 10;
      c[--len] = digit + 48;
    }
  }

  struct hex_type {
    template <typename T>
    static void render(T * c, uint32_t len, uint32_t n)
    {
      hex<T>(c, len, n);
    }
  };

  struct dec_type {
    template <typename T>
    static void render(T * c, uint32_t len, uint32_t n)
    {
      dec<T>(c, len, n);
    }
  };
}
