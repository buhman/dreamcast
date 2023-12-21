#pragma once

#include <cstdint>

struct rect {
  uint32_t char_code;
  uint32_t width;
  uint32_t height;
  int32_t x;
  int32_t y;

  std::strong_ordering operator<=>(const rect& b) const
  {
    return (width * height) <=> (b.width * b.height);
  }
};
