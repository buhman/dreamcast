#include <cstdint>
#include <bit>

#include "align.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "maple_bits.hpp"
#include "maple_bus_bits.hpp"
#include "maple_bus_commands.hpp"
#include "maple.hpp"

namespace maple {

void init_host_command(uint32_t * command_buf, uint32_t * receive_buf,
                       uint32_t destination_port,
                       uint8_t destination_ap, uint8_t command_code, uint8_t data_size,
                       bool end_flag)
{
  // this function does not care about the template instantiation of
  // host_command--data_fields is not manipulated here.
  auto host_command = reinterpret_cast<struct host_command<uint8_t[0]> *>(command_buf);

  host_command->host_instruction = (end_flag ? host_instruction::end_flag : 0)
                                 | (destination_port & host_instruction::port_select::bit_mask) // host_instruction::port_select::a
                                 | host_instruction::transfer_length((data_size / 4));

  host_command->receive_data_storage_address = receive_data_storage_address::address(reinterpret_cast<uint32_t>(receive_buf));

  host_command->bus_data.command_code = command_code;
  host_command->bus_data.destination_ap = destination_ap; //ap::de::expansion_device | ap::port_select::a | ap::lm_bus::_0

  host_command->bus_data.source_ap = destination_ap & ap::port_select::bit_mask;
  host_command->bus_data.data_size = data_size / 4;
}

uint32_t init_device_request(uint32_t * command_buf, uint32_t * receive_buf,
                             uint32_t destination_port,
                             uint8_t destination_ap)
{
  init_host_command(command_buf, receive_buf,
                    destination_port,
                    destination_ap, device_request::command_code, (sizeof (struct device_request::data_fields)),
                    true);

  auto host_command = reinterpret_cast<struct host_command<uint8_t[0]> *>(command_buf);

  return (reinterpret_cast<uint32_t>(&host_command[1]) - reinterpret_cast<uint32_t>(&host_command[0]));
}

uint32_t init_get_condition(uint32_t * command_buf, uint32_t * receive_buf,
                            uint32_t destination_port,
                            uint8_t destination_ap,
                            uint32_t function_type)
{
  init_host_command(command_buf, receive_buf,
                    destination_port,
                    destination_ap, get_condition::command_code, (sizeof (struct get_condition::data_fields)),
                    true);

  auto host_command = reinterpret_cast<struct host_command<get_condition::data_fields> *>(command_buf);

  auto& fields = host_command->bus_data.data_fields;
  // controller function type
  fields.function_type = function_type;

  return (reinterpret_cast<uint32_t>(&host_command[1]) - reinterpret_cast<uint32_t>(&host_command[0]));
}

uint32_t init_block_write(uint32_t * command_buf, uint32_t * receive_buf,
                          uint32_t destination_port,
                          uint8_t destination_ap,
                          uint32_t * data,
                          uint32_t data_size)
{
  using command_type = block_write<uint32_t[0]>;
  init_host_command(command_buf, receive_buf,
                    destination_port,
                    destination_ap, command_type::command_code, (sizeof (struct command_type::data_fields)) + data_size,
                    true);

  auto host_command = reinterpret_cast<struct host_command<command_type::data_fields> *>(command_buf);

  auto& fields = host_command->bus_data.data_fields;
  // BW LCD function type
  fields.function_type = std::byteswap(function_type::bw_lcd);

  // lcd number 0 (1 total lcd)
  fields.pt = 0;

  // phase 0 (from 0 to 3)
  fields.phase = 0;

  // plane 0 (2 total levels of gradation)
  fields.block_no = std::byteswap(0x0000);

  for (uint32_t i = 0; i < (data_size / 4); i++) {
    fields.written_data[i] = data[i];
  }

  return (reinterpret_cast<uint32_t>(&host_command[1]) - reinterpret_cast<uint32_t>(&host_command[0]))
    + data_size;
}

static inline void _dma_start(const uint32_t * command_buf)
{
  using namespace dmac;

  //command_buf = reinterpret_cast<uint32_t *>(reinterpret_cast<uint32_t>(command_buf) | 0xa000'0000);

  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */

  // clear maple-DMA end status
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;

  // disable maple-DMA
  maple_if.MDEN = mden::dma_enable::abort;
  while (mdst::start_status::status(maple_if.MDST) != 0);

  // 20nsec * 0xc350 = 1ms
  constexpr uint32_t one_msec = 0xc350;
  maple_if.MSYS = msys::time_out_counter(one_msec)
                | msys::sending_rate::_2M;

  // top address: the first/lowest address
  // bottom address: the last/highest address
  maple_if.MDAPRO = mdapro::security_code
                  | mdapro::top_address(0x40)
                  | mdapro::bottom_address(0x7f);

  maple_if.MDTSEL = mdtsel::trigger_select::software_initiation;

  maple_if.MDSTAR = mdstar::table_address(reinterpret_cast<uint32_t>(command_buf));

  system.ISTERR = 0xffff'ffff;

  maple_if.MDEN = mden::dma_enable::enable;
  maple_if.MDST = mdst::start_status::start;
}

void dma_start(const uint32_t * command_buf,
               const uint32_t command_size,
               const uint32_t * receive_buf,
               const uint32_t receive_size
               )
{
  // write back operand cache blocks for command buffer prior to starting DMA
  for (uint32_t i = 0; i < align_32byte(command_size) / 32; i++) {
    asm volatile ("ocbwb @%0"
                  :                                                              // output
                  : "r" (reinterpret_cast<uint32_t>(&command_buf[(32 * i) / 4])) // input
                  );
  }

  // start maple DMA
  _dma_start(command_buf);

  // purge operand cache block for receive buffer, prior to returning to the caller
  for (uint32_t i = 0; i < align_32byte(receive_size) / 32; i++) {
    asm volatile ("ocbp @%0"
                  :                                                              // output
                  : "r" (reinterpret_cast<uint32_t>(&receive_buf[(32 * i) / 4])) // input
                  );
  }

  // wait for maple DMA completion
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_MAPLE_DMA) == 0);
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;
}

  // wait for completion
  //while (mdst::start_status::status(maple_if.MDST) != 0);
  /*
  uint32_t last_isterr = 0xffff'ffff;
  uint32_t isterr = 0;
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_MAPLE_DMA) == 0) {
    isterr = system.ISTERR;
    if (isterr != last_isterr) {
      serial::string("maple dma isterr: ");
      serial::integer<uint32_t>(isterr);
      last_isterr = isterr;
    }
  }
  */

}
