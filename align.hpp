#pragma once

#include <cstdint>

template <typename T>
inline T * align_32byte(T * mem)
{
  return reinterpret_cast<T *>((((reinterpret_cast<uint32_t>(mem) + 31) & ~31)));
}
