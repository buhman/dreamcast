#pragma once

#include <stdint.h>

#include "model/material.h"

enum material {
  wn,
};

struct material_descriptor material[] = {
  [wn] = {
    .pixel = {
      .start = (uint8_t *)&_binary_texture_cube_wn_data_start,
      .size = (int)&_binary_texture_cube_wn_data_size,
      .vram_offset = 0,
      .width = 128,
      .height = 128,
    },
    .palette = {
      .start = (uint8_t *)&_binary_texture_cube_wn_data_pal_start,
      .size = (int)&_binary_texture_cube_wn_data_pal_size,
      .vram_offset = 0,
      .palette_size = 256,
    },
  },
};

