#pragma once

#include <cstdint>

namespace maple::ft6 {
  namespace data_transfer {
    namespace modifier_key {
      constexpr uint32_t s2() { return 0b1 << 7; }
      constexpr uint32_t s2(uint32_t reg) { return (reg >> 7) & 0b1; }

      constexpr uint32_t right_alt() { return 0b1 << 6; }
      constexpr uint32_t right_alt(uint32_t reg) { return (reg >> 6) & 0b1; }

      constexpr uint32_t right_shift() { return 0b1 << 5; }
      constexpr uint32_t right_shift(uint32_t reg) { return (reg >> 5) & 0b1; }

      constexpr uint32_t right_control() { return 0b1 << 4; }
      constexpr uint32_t right_control(uint32_t reg) { return (reg >> 4) & 0b1; }

      constexpr uint32_t left_gui() { return 0b1 << 3; }
      constexpr uint32_t left_gui(uint32_t reg) { return (reg >> 3) & 0b1; }

      constexpr uint32_t left_alt() { return 0b1 << 2; }
      constexpr uint32_t left_alt(uint32_t reg) { return (reg >> 2) & 0b1; }

      constexpr uint32_t left_shift() { return 0b1 << 1; }
      constexpr uint32_t left_shift(uint32_t reg) { return (reg >> 1) & 0b1; }

      constexpr uint32_t left_control() { return 0b1 << 0; }
      constexpr uint32_t left_control(uint32_t reg) { return (reg >> 0) & 0b1; }

    }

    namespace led_state {
      constexpr uint32_t shift() { return 0b1 << 7; }
      constexpr uint32_t shift(uint32_t reg) { return (reg >> 7) & 0b1; }

      constexpr uint32_t power() { return 0b1 << 6; }
      constexpr uint32_t power(uint32_t reg) { return (reg >> 6) & 0b1; }

      constexpr uint32_t kana() { return 0b1 << 5; }
      constexpr uint32_t kana(uint32_t reg) { return (reg >> 5) & 0b1; }

      constexpr uint32_t reserved0() { return 0b1 << 4; }
      constexpr uint32_t reserved0(uint32_t reg) { return (reg >> 4) & 0b1; }

      constexpr uint32_t reserved1() { return 0b1 << 3; }
      constexpr uint32_t reserved1(uint32_t reg) { return (reg >> 3) & 0b1; }

      constexpr uint32_t scroll_lock() { return 0b1 << 2; }
      constexpr uint32_t scroll_lock(uint32_t reg) { return (reg >> 2) & 0b1; }

      constexpr uint32_t caps_lock() { return 0b1 << 1; }
      constexpr uint32_t caps_lock(uint32_t reg) { return (reg >> 1) & 0b1; }

      constexpr uint32_t num_lock() { return 0b1 << 0; }
      constexpr uint32_t num_lock(uint32_t reg) { return (reg >> 0) & 0b1; }

    }

    struct data_format {
      uint8_t modifier_key;
      uint8_t led_state;
      uint8_t scan_code_array[6];
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 8);
  }

  namespace set_condition {
    struct data_format {
      uint8_t led_setting;
      uint8_t w1_reserved;
      uint8_t w2_reserved;
      uint8_t w3_reserved;
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 4);
  }

}

