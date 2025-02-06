#pragma once

#include <stdint.h>

#include "model/material.h"

enum material {
  elizabeth_elizabeth_mat,
  elizabeth_elizabeth_sword_mat,
};

const struct material_descriptor elizabeth_material[] = {
  [elizabeth_elizabeth_mat] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_elizabeth_elizabeth_mat_emissive_data_start,
      .size = (int)&_binary_model_elizabeth_elizabeth_mat_emissive_data_size,
      .vram_offset = 0,
      .width = 128,
      .height = 128,
    },
  },
  [elizabeth_elizabeth_sword_mat] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_elizabeth_elizabeth_sword_mat_emissive_data_start,
      .size = (int)&_binary_model_elizabeth_elizabeth_sword_mat_emissive_data_size,
      .vram_offset = 32768,
      .width = 32,
      .height = 64,
    },
  },
};

