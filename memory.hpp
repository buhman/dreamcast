#pragma once

namespace memory {

template <typename T>
void move(T * dst, const T * src, uint32_t n)
{
  if (dst < src) {
    while (n > 0) {
      *dst++ = *src++;
      n--;
    }
  } else {
    while (n > 0) {
      n--;
      dst[n] = src[n];
    }
  }
}

template <typename T>
inline void copy(T * dst, const T * src, uint32_t n)
{
  while (n > 0) {
    *dst++ = *src++;
    n--;
  }
}

}
