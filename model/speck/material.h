#pragma once

#include <stdint.h>

#include "model/material.h"

enum material {
  speck_matSpeck,
  speck_white
};

const struct material_descriptor speck_material[] = {
  [speck_matSpeck] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_speck_speck_data_start,
      .size = (int)&_binary_model_speck_speck_data_size,
      .vram_offset = 0,
      .width = 256,
      .height = 256,
    },
  },
  [speck_white] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_speck_white_data_start,
      .size = (int)&_binary_model_speck_white_data_size,
      .vram_offset = 0,
      .width = 8,
      .height = 8,
    },
  },
};
