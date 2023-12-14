#pragma once

#include <cstdint>

/*
  object_list[0x00100000 / 4] is enough space for 81 sets of
  0x3200-byte lists (16 * 4 * (640 / 32) * (320 / 32))

  (that is, it is significantly more space than required for trivial
  TA/CORE drawings)

  region-array[0x00004000 / 4] is enough space for 2 render passes.
*/

struct texture_memory_alloc {
  uint32_t isp_tsp_parameters[0x00100000 / 4]; // TA_ISP_BASE / PARAM_BASE (the actual objects)
  uint32_t        object_list[0x00100000 / 4]; // TA_OL_BASE (contains object pointer blocks)
  uint32_t              _res0[      0x20 / 4]; // (the TA may clobber 4 bytes starting at TA_OL_LIMIT)
  uint32_t       region_array[0x00004000 / 4]; // REGION_BASE
  uint32_t         background[0x00000040 / 4]; // ISP_BACKGND_T
  uint32_t     framebuffer[2][0x00096000 / 4]; // FB_R_SOF1 / FB_W_SOF1
  uint32_t              _res1[      0x20 / 4]; // (re-align texture to a 64-byte boundary)
  uint16_t         texture[128 * 128 * 2 / 2]; // texture_control_word::texture_address
};

static_assert((sizeof (texture_memory_alloc)) < 0x1000000);
