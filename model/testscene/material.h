#pragma once

#include <stdint.h>

#include "model/material.h"

enum material {
  testscene_matBrick,
  testscene_matFoliage,
  testscene_matGrass,
  testscene_matGrassClump,
  testscene_matWater,
};

const struct material_descriptor testscene_material[] = {
  [testscene_matBrick] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_testscene_texture_texBrick_data_start,
      .size = (int)&_binary_model_testscene_texture_texBrick_data_size,
      .vram_offset = 0,
      .width = 128,
      .height = 128,
    },
  },
  [testscene_matFoliage] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_testscene_texture_texFoliage_data_start,
      .size = (int)&_binary_model_testscene_texture_texFoliage_data_size,
      .vram_offset = 32768,
      .width = 128,
      .height = 128,
    },
  },
  [testscene_matGrass] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_testscene_texture_texGrass_data_start,
      .size = (int)&_binary_model_testscene_texture_texGrass_data_size,
      .vram_offset = 65536,
      .width = 128,
      .height = 128,
    },
  },
  [testscene_matGrassClump] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_testscene_texture_texGrassClump_data_start,
      .size = (int)&_binary_model_testscene_texture_texGrassClump_data_size,
      .vram_offset = 98304,
      .width = 128,
      .height = 128,
    },
  },
  [testscene_matWater] = {
    .pixel = {
      .start = (uint8_t *)&_binary_model_testscene_texture_texWater_data_start,
      .size = (int)&_binary_model_testscene_texture_texWater_data_size,
      .vram_offset = 131072,
      .width = 128,
      .height = 128,
    },
  },
};

