#pragma once

#include <stdint.h>

#include "model/material.h"

enum material {
  body,
  eyes,
};

struct material_descriptor material[] = {
  [body] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_sheik_sheik_00_data_start,
      .size = (int)&_binary_model_sheik_sheik_00_data_size,
      .vram_offset = 0,
      .width = 128,
      .height = 256,
    },
  },
  [eyes] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_sheik_xc_eye01_data_start,
      .size = (int)&_binary_model_sheik_xc_eye01_data_size,
      .vram_offset = 65536,
      .width = 32,
      .height = 32,
    },
  },
};

