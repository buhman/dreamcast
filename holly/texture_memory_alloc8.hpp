#pragma once

#include <cstdint>
#include <cstddef>

struct texture_memory_alloc__start_end {
  uint32_t start;
  uint32_t end;
};

struct texture_memory_alloc {
  struct texture_memory_alloc__start_end isp_tsp_parameters;
  struct texture_memory_alloc__start_end object_list;
  struct texture_memory_alloc__start_end region_array;
  struct texture_memory_alloc__start_end framebuffer[2];
  struct texture_memory_alloc__start_end background;
  struct texture_memory_alloc__start_end texture;
};

constexpr texture_memory_alloc texture_memory_alloc = {
  // 32-bit addresses     start      end          start      end
  .isp_tsp_parameters = {0x400000, 0x61bfe0},
  .object_list = {0x0a8c00, 0x13ebe0},
  .region_array = {0x61c000, 0x62c000},
  .framebuffer = {{0x62c000, 0x6d4be0},
                  {0x000000, 0x0a8be0}},
  .background = {0x13ec00, 0x13ec20},

  // 64-bit addresses
  .texture = {0x458000, 0x800000},
};
