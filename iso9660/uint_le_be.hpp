#pragma once

#include <cstdint>
#include <bit>

template <typename T>
struct uint_le_be {
  const uint8_t le[(sizeof (T))];
  const uint8_t be[(sizeof (T))];

  T get() const {
    union {
      uint8_t u8[(sizeof (T))];
      T uint;
    } value;
    static_assert((sizeof (value)) == (sizeof (T)));

    if constexpr (std::endian::native == std::endian::little) {
      for (uint32_t i = 0; i < (sizeof (T)); i++) {
	value.u8[i] = le[i];
      }
      return value.uint;
    } else {
      for (uint32_t i = 0; i < (sizeof (T)); i++) {
	value.u8[i] = be[i];
      }
      return value.uint;
    }
  }
};

using uint16_le_be = uint_le_be<uint16_t>;
using uint32_le_be = uint_le_be<uint32_t>;

static_assert((sizeof (uint16_le_be)) == 4);
static_assert((sizeof (uint32_le_be)) == 8);
