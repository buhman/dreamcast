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
static_assert((sizeof (host_command<uint8_t[0]>)) == 12);
static_assert((sizeof (host_command<uint8_t[0]>[4])) == 48);

template <typename T>
struct host_response {
  struct bus_data {
    uint8_t command_code;
    uint8_t destination_ap;
    uint8_t source_ap;
    uint8_t data_size;
    T data_fields;
  } bus_data;
  uint8_t _pad[align_32byte((sizeof (bus_data))) - (sizeof (bus_data))];
};
static_assert((sizeof (host_response<uint8_t[0]>)) == align_32byte((sizeof (host_response<uint8_t[0]>))));

void init_host_command(uint32_t * buf, uint32_t * receive_buf,
                       uint32_t destination_port,
                       uint8_t destination_ap, uint8_t command_code, uint8_t data_size,
                       bool end_flag);

uint32_t init_device_request(uint32_t * buf, uint32_t * receive_buf,
                             uint32_t destination_port,
                             uint8_t destination_ap);

uint32_t init_get_condition(uint32_t * buf, uint32_t * receive_buf,
                            uint32_t destination_port,
                            uint8_t destination_ap,
                            uint32_t function_type);

uint32_t init_block_write(uint32_t * buf, uint32_t * receive_buf,
                          uint32_t destination_port,
                          uint8_t destination_ap,
                          uint32_t * data,
                          uint32_t data_size);

void dma_start(const uint32_t * command_buf,
               const uint32_t command_size,
               const uint32_t * receive_buf,
               const uint32_t receive_size
               );

template <typename T>
constexpr uint32_t sizeof_command(T * c)
{
  return reinterpret_cast<uint32_t>(&c[1]) - reinterpret_cast<uint32_t>(&c[0]);
}

}
