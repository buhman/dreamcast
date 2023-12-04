#pragma once

#include <cstdint>

struct texture_memory_alloc {
  uint32_t isp_tsp_parameters[0x00100000 / 4]; // TA_ISP_BASE / PARAM_BASE (the actual objects)
  uint32_t        object_list[0x00100000 / 4]; // TA_OL_BASE (contains object pointer blocks)
  uint32_t              _res0[      0x20 / 4]; // (the TA may clobber 4 bytes starting at TA_OL_LIMIT)
  uint32_t       region_array[0x00002000 / 4]; // REGION_BASE
  uint32_t         background[0x00000020 / 4]; // ISP_BACKGND_T
  uint32_t     framebuffer[2][0x00096000 / 4]; // FB_R_SOF1 / FB_W_SOF1
};
