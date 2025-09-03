#pragma once

#include <cstdint>

namespace maple::ft0 {
  namespace data_transfer {
    namespace digital_button {
      constexpr uint32_t ra() { return 0b1 << 7; }
      constexpr uint32_t ra(uint32_t reg) { return (reg >> 7) & 0b1; }

      constexpr uint32_t la() { return 0b1 << 6; }
      constexpr uint32_t la(uint32_t reg) { return (reg >> 6) & 0b1; }

      constexpr uint32_t da() { return 0b1 << 5; }
      constexpr uint32_t da(uint32_t reg) { return (reg >> 5) & 0b1; }

      constexpr uint32_t ua() { return 0b1 << 4; }
      constexpr uint32_t ua(uint32_t reg) { return (reg >> 4) & 0b1; }

      constexpr uint32_t start() { return 0b1 << 3; }
      constexpr uint32_t start(uint32_t reg) { return (reg >> 3) & 0b1; }

      constexpr uint32_t a() { return 0b1 << 2; }
      constexpr uint32_t a(uint32_t reg) { return (reg >> 2) & 0b1; }

      constexpr uint32_t b() { return 0b1 << 1; }
      constexpr uint32_t b(uint32_t reg) { return (reg >> 1) & 0b1; }

      constexpr uint32_t c() { return 0b1 << 0; }
      constexpr uint32_t c(uint32_t reg) { return (reg >> 0) & 0b1; }

      constexpr uint32_t rb() { return 0b1 << 15; }
      constexpr uint32_t rb(uint32_t reg) { return (reg >> 15) & 0b1; }

      constexpr uint32_t lb() { return 0b1 << 14; }
      constexpr uint32_t lb(uint32_t reg) { return (reg >> 14) & 0b1; }

      constexpr uint32_t db() { return 0b1 << 13; }
      constexpr uint32_t db(uint32_t reg) { return (reg >> 13) & 0b1; }

      constexpr uint32_t ub() { return 0b1 << 12; }
      constexpr uint32_t ub(uint32_t reg) { return (reg >> 12) & 0b1; }

      constexpr uint32_t d() { return 0b1 << 11; }
      constexpr uint32_t d(uint32_t reg) { return (reg >> 11) & 0b1; }

      constexpr uint32_t x() { return 0b1 << 10; }
      constexpr uint32_t x(uint32_t reg) { return (reg >> 10) & 0b1; }

      constexpr uint32_t y() { return 0b1 << 9; }
      constexpr uint32_t y(uint32_t reg) { return (reg >> 9) & 0b1; }

      constexpr uint32_t z() { return 0b1 << 8; }
      constexpr uint32_t z(uint32_t reg) { return (reg >> 8) & 0b1; }

    }

    struct data_format {
      uint16_t digital_button;
      uint8_t analog_coordinate_axis[6];
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 8);
  }

}

