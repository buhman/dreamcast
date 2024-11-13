#pragma once

#include <stdint.h>
#include <ftdi.h>

int do_maple_raw(struct ftdi_context * ftdi,
                 uint8_t * send_buf,
                 uint32_t send_size,
                 uint8_t * recv_buf,
                 uint32_t recv_size);
