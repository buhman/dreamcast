#pragma once

#include <cstdint>

namespace holly::core::object_list {
  namespace pointer_type {
    constexpr uint32_t triangle_strip = 0b000 << 29;
    constexpr uint32_t triangle_array = 0b100 << 29;
    constexpr uint32_t quad_array = 0b101 << 29;
    constexpr uint32_t object_pointer_block_link = 0b111 << 29;

    constexpr uint32_t bit_mask = 0x7 << 29;
  }

  namespace triangle_strip {
    namespace mask {
      constexpr uint32_t t0 = 0b100000 << 25;
      constexpr uint32_t t1 = 0b010000 << 25;
      constexpr uint32_t t2 = 0b001000 << 25;
      constexpr uint32_t t3 = 0b000100 << 25;
      constexpr uint32_t t4 = 0b000010 << 25;
      constexpr uint32_t t5 = 0b000001 << 25;

      constexpr uint32_t bit_mask = 0x3f << 25;
    }

    constexpr uint32_t shadow = 1 << 24;
    constexpr inline uint32_t skip(uint32_t num) { return (num & 0x7) << 21; }
    constexpr inline uint32_t start(uint32_t num) { return (num & 0x1fffff) << 0; }
  }

  namespace triangle_array {
    constexpr inline uint32_t number_of_triangles(uint32_t num) { return (num & 0xf) << 25; }
    constexpr uint32_t shadow = 1 << 24;
    constexpr inline uint32_t skip(uint32_t num) { return (num & 0x7) << 21; }
    constexpr inline uint32_t start(uint32_t num) { return (num & 0x1fffff) << 0; }
  }

  namespace quad_array {
    constexpr inline uint32_t number_of_quads(uint32_t num) { return (num & 0xf) << 25; }
    constexpr uint32_t shadow = 1 << 24;
    constexpr inline uint32_t skip(uint32_t num) { return (num & 0x7) << 21; }
    constexpr inline uint32_t start(uint32_t num) { return (num & 0x1fffff) << 0; }
  }

  namespace object_pointer_block_link {
    constexpr uint32_t end_of_list = 1 << 28;
    constexpr inline uint32_t next_pointer_block(uint32_t num) { return (num & 0xfffffc) << 0; }
  }

}
