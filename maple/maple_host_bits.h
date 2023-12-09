#include <cstdint>

#include "../float_uint32.h"

namespace host_instruction {
  constexpr uint32_t end_flag = 1 << 31;
  
  namespace port_select {
    constexpr uint32_t a = 0 << 16;
    constexpr uint32_t b = 1 << 16;
    constexpr uint32_t c = 2 << 16;
    constexpr uint32_t d = 3 << 16;
  }
  
  namespace pattern {
    constexpr uint32_t normal = 0b000 << 8;
    constexpr uint32_t light_gun_mode = 0b010 << 8;
    constexpr uint32_t reset = 0b011 << 8;
    constexpr uint32_t return_from_light_gun_mode = 0b100 << 8;
    constexpr uint32_t nop = 0b111 << 8;
  }
  
  constexpr uint32_t transfer_length(uint32_t num) { return (num & 0xff) << 0; }
}

namespace ap {
  namespace port_select {
    constexpr uint32_t a = 0b00 << 6;
    constexpr uint32_t b = 0b01 << 6;
    constexpr uint32_t c = 0b10 << 6;
    constexpr uint32_t d = 0b11 << 6;
  }
  
  namespace de {
    constexpr uint32_t device = 1 << 5;
    constexpr uint32_t expansion_device = 0 << 5;
    constexpr uint32_t port = 0 << 5;
  }
  
  namespace lm_bus {
    constexpr uint32_t _4 = 0b10000 << 0;
    constexpr uint32_t _3 = 0b01000 << 0;
    constexpr uint32_t _2 = 0b00100 << 0;
    constexpr uint32_t _1 = 0b00010 << 0;
    constexpr uint32_t _0 = 0b00001 << 0;
  }
}

