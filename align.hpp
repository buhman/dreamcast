#pragma once

#include <cstdint>

template <typename T>
constexpr inline T * align_32byte(T * mem)
{
  return reinterpret_cast<T *>((((reinterpret_cast<uint32_t>(mem) + 31) & ~31)));
}

constexpr inline uint32_t align_32byte(uint32_t mem)
{
  return (mem + 31) & ~31;
}
