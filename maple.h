#pragma once

#include <cstdint>

void maple_init_host_command(uint32_t * buf, uint32_t * receive_address);
void maple_dma_start(uint32_t * command_buf);
