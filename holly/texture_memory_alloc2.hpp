#pragma once

#include <cstdint>
#include <cstddef>

/*
 * A 0x10000-byte region array is sufficient for 9 render passes:
 *
 *   ((640 / 32) * (480 / 32) * 6 * 4) * 9 == 0xfd20
 */

namespace texture_memory_alloc {

constexpr uint32_t address_64to32(uint32_t addr)
{
  uint32_t value = ((addr & 0xfffffff8) >> 1) + (addr & 0x3);
  if ((addr & 0x4) != 0)
    value += 0x400000;
  if (addr >= 0x800000)
    value += 0x400000;
  return value;
}

struct start_end {
  uint32_t start;
  uint32_t end;
};

constexpr start_end isp_tsp_parameters[2] = {
  { // bus A
    .start = 0x00'0000,
    .end   = 0x0f'ffc0,
  },
  { // bus B
    .start = 0x40'0000,
    .end   = 0x4f'ffc0,
  },
};
constexpr start_end background[2] = {
  {
    .start = 0x0f'ffc0,
    .end   = 0x10'0000,
  },
  {
    .start = 0x4f'ffc0,
    .end   = 0x50'0000,
  },
};
constexpr start_end object_list[2] = {
  { // bus A
    .start = 0x10'0000,
    .end   = 0x20'0000 - 0x10,
  },
  { // bus B
    .start = 0x50'0000,
    .end   = 0x60'0000 - 0x10,
  }
};
constexpr start_end framebuffer[2] = {
  { // bus A
    .start = 0x20'0000,
    .end   = 0x29'6000,
  },
  { // bus B
    .start = 0x60'0000,
    .end   = 0x69'6000,
  }
};
constexpr start_end region_array[2] = {
  { // bus A
    .start = 0x29'6000,
    .end   = 0x2a'6000,
  },
  { // bus B
    .start = 0x69'6000,
    .end   = 0x6a'6000,
  },
};

// 64-bit transfer
constexpr start_end texture = {
  .start = 0x54'c000,
  .end   = 0x80'0000,
};

}
