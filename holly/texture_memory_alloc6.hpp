#pragma once

#include <cstdint>
#include <cstddef>

struct texture_memory_alloc__start_end {
  uint32_t start;
  uint32_t end;
};

struct texture_memory_alloc {
  struct texture_memory_alloc__start_end isp_tsp_parameters[2];
  struct texture_memory_alloc__start_end object_list[2];
  struct texture_memory_alloc__start_end region_array[2];
  struct texture_memory_alloc__start_end framebuffer[2];
  struct texture_memory_alloc__start_end background[2];
  struct texture_memory_alloc__start_end texture;
};

constexpr texture_memory_alloc texture_memory_alloc = {
  // 32-bit addresses     start      end          start      end
  .isp_tsp_parameters = {0x000000, 0x11bfe0, 0x400000, 0x51bfe0},
  .object_list        = {0x11c000, 0x166fe0, 0x51c000, 0x566fe0},
  .region_array       = {0x167000, 0x177000, 0x567000, 0x577000},
  .framebuffer        = {0x177000, 0x18b000, 0x577000, 0x58b000},
  .background         = {0x18b000, 0x18b020, 0x58b000, 0x58b020},
  // 64-bit addresses
  .texture            = 0x316040,
};
