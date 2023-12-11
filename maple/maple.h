#pragma once

#include <cstdint>

template <typename T>
struct maple_host_command {
  uint32_t host_instruction;
  uint32_t receive_data_storage_address;
  struct bus_data {
    uint8_t command_code;
    uint8_t destination_ap;
    uint8_t source_ap;
    uint8_t data_size;
    T data_fields;
  } bus_data;
};

void maple_init_host_command(uint32_t * buf, uint32_t * receive_address);
void maple_init_device_request(uint32_t * buf, uint32_t * receive_address);
void maple_init_get_condition(uint32_t * buf, uint32_t * receive_address);
void maple_init_block_write(uint32_t * buf, uint32_t * receive_address, uint32_t * data);
void maple_dma_start(uint32_t * command_buf);
