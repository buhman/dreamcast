#pragma once

#include <stdint.h>

struct pk_texture {
  void * start;
  uint32_t size;
  uint32_t offset;
  int16_t width;
  int16_t height;
  //float u_mul;
  float v_mul;
};

struct pk_texture textures[] = {
  #include "texture.inc"
};
