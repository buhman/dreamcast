#pragma once

#include <cstdint>

namespace holly::core::region_array {
  namespace tile {
    constexpr uint32_t last_region = 1 << 31;
    constexpr uint32_t z_clear = 1 << 30;
    constexpr uint32_t pre_sort = 1 << 29;
    constexpr uint32_t flush_accumulate = 1 << 28;
    constexpr inline uint32_t y_position(uint32_t num) { return (num & 0x3f) << 8; }
    constexpr inline uint32_t x_position(uint32_t num) { return (num & 0x3f) << 2; }
  }

  namespace list_pointer {
    constexpr uint32_t empty = 1 << 31;
    constexpr inline uint32_t object_list(uint32_t num) { return (num & 0xfffffc) << 0; }
  }

}
