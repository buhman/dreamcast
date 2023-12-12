#pragma once

#include <cstdint>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
constexpr uint32_t _i(float f) {
  return *(reinterpret_cast<uint32_t *>(&f));
}
#pragma GCC diagnostic pop
