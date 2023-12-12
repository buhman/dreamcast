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

void init_host_command(uint32_t * buf, uint32_t * receive_buf, uint8_t command_code, uint8_t data_size)
{
  // this function does not care about the template instantiation of
  // host_command--data_fields is not manipulated here.
  auto host_command = reinterpret_cast<struct host_command<uint32_t> *>(buf);

  host_command->host_instruction = host_instruction::end_flag
				 | host_instruction::port_select::a
				 | host_instruction::transfer_length((data_size / 4));

  host_command->receive_data_storage_address = reinterpret_cast<uint32_t>(receive_buf) & 0x1fff'ffff;

  host_command->bus_data.command_code = command_code;
  host_command->bus_data.destination_ap = ap::de::expansion_device | ap::port_select::a | ap::lm_bus::_0;
  host_command->bus_data.source_ap = ap::port_select::a;
  host_command->bus_data.data_size = data_size / 4;
}

void init_device_request(uint32_t * buf, uint32_t * receive_buf)
{
  init_host_command(buf, receive_buf, device_request::command_code, (sizeof (struct device_request::data_fields)));
}

void init_get_condition(uint32_t * buf, uint32_t * receive_buf)
{
  init_host_command(buf, receive_buf, get_condition::command_code, (sizeof (struct get_condition::data_fields)));

  auto host_command = reinterpret_cast<struct host_command<get_condition::data_fields> *>(buf);

  auto& fields = host_command->bus_data.data_fields;
  // controller function type
  fields.function_type = std::byteswap(function_type::controller);
}

void init_block_write(uint32_t * buf, uint32_t * receive_buf, uint32_t * data)
{
  init_host_command(buf, receive_buf, block_write::command_code, (sizeof (struct block_write::data_fields<uint32_t[192 / 4]>)));

  auto host_command = reinterpret_cast<struct host_command<block_write::data_fields<uint32_t[192 / 4]>> *>(buf);

  auto& fields = host_command->bus_data.data_fields;
  // BW LCD function type
  fields.function_type = std::byteswap(function_type::bw_lcd);

  // lcd number 0 (1 total lcd)
  fields.pt = 0;

  // phase 0 (from 0 to 3)
  fields.phase = 0;

  // plane 0 (2 total levels of gradation)
  fields.block_no = std::byteswap(0x0000);

  for (uint32_t i = 0; i < (192 / 4); i++) {
    fields.written_data[i] = data[i];
  }
}

void dma_start(uint32_t * command_buf)
{
  sh7091.DMAC.DMAOR = DMAOR__DDT                 /* on-demand data transfer mode */
    | DMAOR__PR__CH2_CH0_CH1_CH3 /* priority mode; CH2 > CH0 > CH1 > CH3 */
    | DMAOR__DME;                /* DMAC master enable */

  // clear maple-DMA end status
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;

  // disable maple-DMA
  maple_if.MDEN = mden::dma_enable::abort;

  while (mdst::start_status::status(maple_if.MDST) != 0);

  // 20nsec * 0xc350 = 1ms
  constexpr uint32_t one_msec = 0xc350;
  maple_if.MSYS = msys::time_out_counter(one_msec)
    | msys::sending_rate::_2M;

  /* top address: the first/lowest address
     bottom address: the last/highest address */
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
