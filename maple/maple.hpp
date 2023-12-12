#pragma once

#include <cstdint>

namespace maple {

template <typename T>
struct host_command {
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

void init_host_command(uint32_t * command_buf, uint32_t * receive_buf);
void init_device_request(uint32_t * command_buf, uint32_t * receive_buf);
void init_get_condition(uint32_t * command_buf, uint32_t * receive_buf);
void init_block_write(uint32_t * command_buf, uint32_t * receive_buf, uint32_t * data);
void dma_start(uint32_t * command_buf);

}

