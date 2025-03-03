#pragma once

#include <stdint.h>

constexpr inline int32_t min(int32_t a, int32_t b)
{
  return (a < b) ? a : b;
}

constexpr inline int32_t max(int32_t a, int32_t b)
{
  return (a > b) ? a : b;
}
