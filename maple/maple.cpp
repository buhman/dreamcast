#include <cstdint>
#include <bit>

#include "../sh7091.hpp"
#include "../sh7091_bits.hpp"
#include "../systembus.hpp"
#include "../systembus_bits.hpp"

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

void init_host_command_all_ports(uint32_t * buf, uint32_t * receive_buf,
                                 uint8_t command_code, uint32_t command_data_size, uint32_t response_data_size)
{
  const uint32_t command_size = (((sizeof (struct host_command<uint8_t[0]>)) + command_data_size));
  const uint32_t response_size = (((sizeof (struct command_response<uint8_t[0]>)) + response_data_size) + 31) & ~31;

  init_host_command(&buf[(command_size / 4) * 0], &receive_buf[(response_size / 4) * 0],
                    host_instruction::port_select::a, // destination_port
                    ap::de::device | ap::port_select::a, command_code, command_data_size,
                    false); // end_flag

  init_host_command(&buf[(command_size / 4) * 1], &receive_buf[(response_size / 4) * 1],
                    host_instruction::port_select::b, // destination_port
                    ap::de::device | ap::port_select::b, command_code, command_data_size,
                    false); // end_flag

  init_host_command(&buf[(command_size / 4) * 2], &receive_buf[(response_size / 4) * 2],
                    host_instruction::port_select::c, // destination_port
                    ap::de::device | ap::port_select::c, command_code, command_data_size,
                    false); // end_flag

  init_host_command(&buf[(command_size / 4) * 3], &receive_buf[(response_size / 4) * 3],
                    host_instruction::port_select::d, // destination_port
                    ap::de::device | ap::port_select::d, command_code, command_data_size,
                    true); // end_flag
}

void init_device_request(uint32_t * buf, uint32_t * receive_buf,
                         uint32_t destination_port,
                         uint8_t destination_ap)
{
  init_host_command(buf, receive_buf,
                    destination_port,
                    destination_ap, device_request::command_code, (sizeof (struct device_request::data_fields)),
                    true);
}

void init_get_condition(uint32_t * buf, uint32_t * receive_buf,
                        uint32_t destination_port,
                        uint8_t destination_ap)
{
  init_host_command(buf, receive_buf,
                    destination_port,
                    destination_ap, get_condition::command_code, (sizeof (struct get_condition::data_fields)),
                    true);

  auto host_command = reinterpret_cast<struct host_command<get_condition::data_fields> *>(buf);

  auto& fields = host_command->bus_data.data_fields;
  // controller function type
  fields.function_type = std::byteswap(function_type::controller);
}

void init_block_write(uint32_t * command_buf, uint32_t * receive_buf,
                      uint32_t destination_port,
                      uint8_t destination_ap,
                      uint32_t * data,
                      uint32_t data_size)
{
  init_host_command(command_buf, receive_buf,
                    destination_port,
                    destination_ap, block_write::command_code, (sizeof (struct block_write::data_fields<uint8_t[0]>)) + data_size,
                    true);

  auto host_command = reinterpret_cast<struct host_command<block_write::data_fields<uint8_t[0]>> *>(command_buf);

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
}

void dma_start(uint32_t * command_buf)
{
  sh7091.DMAC.DMAOR = DMAOR__DDT                 // on-demand data transfer mode
                    | DMAOR__PR__CH2_CH0_CH1_CH3 // priority mode; CH2 > CH0 > CH1 > CH3
                    | DMAOR__DME;                // DMAC master enable

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
    | mdapro::top_address(0x00)
    | mdapro::bottom_address(0x7f);

  maple_if.MDTSEL = mdtsel::trigger_select::software_initiation;

  maple_if.MDSTAR = mdstar::table_address(reinterpret_cast<uint32_t>(command_buf));

  maple_if.MDEN = mden::dma_enable::enable;
  maple_if.MDST = mdst::start_status::start;

  // wait for completion
  //while (mdst::start_status::status(maple_if.MDST) != 0);
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_MAPLE_DMA) == 0);
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;
}

}
