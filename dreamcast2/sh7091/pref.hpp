#pragma once

#define pref(address) \
  { asm volatile ("pref @%0" : : "r" ((uint32_t)(address)) : "memory"); }

namespace sh7091::ccn::qacr0 {
  template <typename T>
  constexpr inline uint32_t address(T a) { return (((uint32_t)a) >> 24) & 0b11100; }
}

namespace sh7091::ccn::qacr1 {
  template <typename T>
  constexpr inline uint32_t address(T a) { return (((uint32_t)a) >> 24) & 0b11100; }
}
