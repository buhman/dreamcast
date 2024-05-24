#pragma once

namespace ft9 {
  namespace data_transfer {
    namespace button {
      constexpr uint32_t r() { return 0b1 << 7; }
      constexpr uint32_t r(uint32_t reg) { return (reg >> 7) & 0b1; }

      constexpr uint32_t l() { return 0b1 << 6; }
      constexpr uint32_t l(uint32_t reg) { return (reg >> 6) & 0b1; }

      constexpr uint32_t d() { return 0b1 << 5; }
      constexpr uint32_t d(uint32_t reg) { return (reg >> 5) & 0b1; }

      constexpr uint32_t u() { return 0b1 << 4; }
      constexpr uint32_t u(uint32_t reg) { return (reg >> 4) & 0b1; }

      constexpr uint32_t s() { return 0b1 << 3; }
      constexpr uint32_t s(uint32_t reg) { return (reg >> 3) & 0b1; }

      constexpr uint32_t a() { return 0b1 << 2; }
      constexpr uint32_t a(uint32_t reg) { return (reg >> 2) & 0b1; }

      constexpr uint32_t b() { return 0b1 << 1; }
      constexpr uint32_t b(uint32_t reg) { return (reg >> 1) & 0b1; }

      constexpr uint32_t c() { return 0b1 << 0; }
      constexpr uint32_t c(uint32_t reg) { return (reg >> 0) & 0b1; }

    }

    namespace option {
      constexpr uint32_t batt() { return 0b1 << 1; }
      constexpr uint32_t batt(uint32_t reg) { return (reg >> 1) & 0b1; }

      constexpr uint32_t wire() { return 0b1 << 0; }
      constexpr uint32_t wire(uint32_t reg) { return (reg >> 0) & 0b1; }

    }

    struct data_format {
      uint8_t button;
      uint8_t option;
      uint8_t analog_coordinate_overflow;
      uint8_t reserved;
      uint16_t analog_coordinate_axis[8];
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 20);
  }

}

