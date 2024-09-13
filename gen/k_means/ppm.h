#pragma once

#include <stdint.h>

struct ppm_header {
  int width;
  int height;
  int colors;
  uint8_t * data;
  int length;
};

int ppm_parse(uint8_t * buf, int size, struct ppm_header * out);
