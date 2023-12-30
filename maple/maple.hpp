#pragma once

#include <cstdint>

#include "align.hpp"

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

template <typename T>
struct command_response {
  struct bus_data {
    uint8_t command_code;
    uint8_t destination_ap;
    uint8_t source_ap;
    uint8_t data_size;
    T data_fields;
  } bus_data;
  uint8_t _pad[align_32byte((sizeof (bus_data))) - (sizeof (bus_data))];
};
static_assert((sizeof (command_response<uint8_t[0]>)) == align_32byte((sizeof (command_response<uint8_t[0]>))));

void init_host_command(uint32_t * buf, uint32_t * receive_buf,
                       uint32_t destination_port,
                       uint8_t destination_ap, uint8_t command_code, uint8_t data_size,
                       bool end_flag);

void init_device_request(uint32_t * buf, uint32_t * receive_buf,
                         uint32_t destination_port,
                         uint8_t destination_ap);

void init_get_condition(uint32_t * buf, uint32_t * receive_buf,
                        uint32_t destination_port,
                        uint8_t destination_ap,
			uint32_t function_type);

void init_block_write(uint32_t * buf, uint32_t * receive_buf,
                      uint32_t destination_port,
                      uint8_t destination_ap,
                      uint32_t * data,
                      uint32_t data_size);

void dma_start(uint32_t * command_buf);

}
