#pragma once

#include <cstdint>
#include <cstddef>

/*
 * A 0x10000-byte region array is sufficient for 9 render passes:
 *
 *   ((640 / 32) * (480 / 32) * 6 * 4) * 9 == 0xfd20
 */

struct texture_memory_alloc__start_end {
  uint32_t start;
  uint32_t end;
};

struct texture_memory_alloc {
  struct texture_memory_alloc__start_end isp_tsp_parameters[2];
  struct texture_memory_alloc__start_end background[2];
  struct texture_memory_alloc__start_end object_list[2];
  struct texture_memory_alloc__start_end region_array[2];
  struct texture_memory_alloc__start_end framebuffer[2];
  struct texture_memory_alloc__start_end texture;
};

constexpr texture_memory_alloc texture_memory_alloc = {
  //                      bus a                   bus b
  // 32-bit addresses     start      end          start      end
  .isp_tsp_parameters = {{0x00'0000, 0x07'ffe0}, {0x40'0000, 0x47'ffe0}}, // 5461 textured triangles
  .background         = {{0x07'ffe0, 0x08'0000}, {0x47'ffe0, 0x48'0000}},
  .object_list        = {{0x08'0000, 0x0f'ffe0}, {0x48'0000, 0x4f'ffe0}}, // ~122880 object list pointers
  .region_array       = {{0x10'0000, 0x11'0000}, {0x50'0000, 0x51'0000}}, // ~9 render passes
  .framebuffer        = {{0x11'0000, 0x1b'8c00}, {0x51'0000, 0x5b'8c00}}, // 720x480*2
  //.framebuffer        = {{0x11'0000, 0x23'c000}, {0x51'0000, 0x63'c000}}, // 640x480*4

  // 64-bit addresses
  .texture = {0x37'1800, 0x80'0000}
  //.texture = {0x57'1800, 0x80'0000}
};
