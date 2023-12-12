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
}
