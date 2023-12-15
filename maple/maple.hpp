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

template <typename T>
struct command_response {
  struct bus_data {
    uint8_t command_code;
    uint8_t destination_ap;
    uint8_t source_ap;
    uint8_t data_size;
    T data_fields;
  } bus_data;
};

void init_host_command(uint32_t * buf, uint32_t * receive_buf,
                       uint32_t destination_port,
                       uint8_t destination_ap, uint8_t command_code, uint8_t data_size,
                       bool end_flag);

void init_host_command_all_ports(uint32_t * buf, uint32_t * receive_buf,
                                 uint8_t command_code, uint32_t command_data_size, uint32_t response_data_size);

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
