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

// 32-bit transfer
constexpr start_end isp_tsp_parameters = {
  .start = address_64to32(0x00'0000),
  .end   = address_64to32(0x00'0000) + 0x10'0000,        // next: 0x20'0000
};
constexpr start_end object_list = {
  .start = address_64to32(0x00'0004),
  .end   = address_64to32(0x00'0004) + 0x10'0000 - 0x20, // next: 0x20'0004
};
constexpr start_end framebuffer[2] = {
  {
    .start = address_64to32(0x20'0000),
    .end   = address_64to32(0x20'0000) + 0x9'6000, // next: 0x32'c000
  },
  {
    .start = address_64to32(0x20'0004),
    .end   = address_64to32(0x20'0004) + 0x9'6000, // next: 0x32'c004
  }
};
constexpr start_end background = {
  .start = address_64to32(0x32'c000),
  .end   = address_64to32(0x32'c000) + 0x40,     // next: 0x32'c080
};
constexpr start_end region_array = {
  .start = address_64to32(0x32'c080),
  .end   = address_64to32(0x32'c080) + 0x1'0000, // next: 0x34'c080
};

// 64-bit transfer
constexpr start_end texture = {
  .start = 0x40'0000,
  .end   = 0x80'0000,
};

}

/*
  struct texture_memory_alloc {
  union {
  struct {
  uint32_t isp_tsp_parameters[0x0010'0000 / 4]; // TA_ISP_BASE / PARAM_BASE (the actual objects)
  uint32_t        object_list[0x0010'0000 / 4]; // TA_OL_BASE (contains object pointer blocks)
  uint32_t              _res0[      0x20 * 2 / 4]; // (the TA may clobber 4 bytes starting at TA_OL_LIMIT)
  uint32_t         background[0x00000040 * 2 / 4]; // ISP_BACKGND_T
  uint32_t       region_array[0x00004000 * 2 / 4]; // REGION_BASE
  uint32_t     framebuffer[2][0x0009'6000 * 2 / 4]; // FB_R_SOF1 / FB_W_SOF1
  };
  uint32_t bank0[0x400000 / 4];
  };
  uint16_t              texture[0x200000 / 2]; // texture_control_word::texture_address
  };
  static_assert((sizeof (texture_memory_alloc)) <= 0x800000);
  //static_assert((offsetof (struct texture_memory_alloc, texture)) == 0x400000);
  */
