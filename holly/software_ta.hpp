#pragma once

#include <stdint.h>

struct ta_configuration {
  int32_t isp_base;      // bytes
  int32_t isp_limit;     // bytes
  int32_t ol_base;       // bytes
  int32_t ol_limit;      // bytes
  int32_t alloc_ctrl;
  int32_t next_opb_init; // bytes
  uint8_t tile_x_num;
  uint8_t tile_y_num;
};

void software_ta_init(const struct ta_configuration * config);

void software_ta_transfer(void * src, int32_t src_size,
                          void * dst);
