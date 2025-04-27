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
  struct texture_memory_alloc__start_end framebuffer[3];
  struct texture_memory_alloc__start_end background[2];
  struct texture_memory_alloc__start_end texture;
};

constexpr texture_memory_alloc texture_memory_alloc = {
  // 32-bit addresses     start      end          start      end
  .isp_tsp_parameters =  {0x000000, 0x11bfe0},
  .object_list        =  {0x400000, 0x495fe0},
  .region_array       =  {0x11c000, 0x12c000},
  .framebuffer        = {{0x496000, 0x52c000},
                         {0x12c000, 0x1c2000},
                         {0x52c000, 0x5c2000}},
  .background         = {{0x1c2000, 0x1c2020},
                         {0x5c2000, 0x5c2020}},
  // 64-bit addresses
  .texture            =  {0x384040, 0x800000}
};
