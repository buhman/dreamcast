#include <cstdint>
#include <bit>

#include "../sh7091.h"
#include "../sh7091_bits.h"
#include "../systembus.h"
#include "../systembus_bits.h"

#include "maple_bits.h"
#include "maple_host_bits.h"
#include "maple_bus_commands.h"
#include "maple.h"

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

void maple_init_host_command(uint32_t * buf, uint32_t * receive_address)
{
  auto host_command = reinterpret_cast<maple_host_command<device_request::data_fields> *>(buf);

  host_command->host_instruction = host_instruction::end_flag
                                 | host_instruction::port_select::a
                                 | host_instruction::transfer_length(0); // 4 bytes

  host_command->receive_data_storage_address = reinterpret_cast<uint32_t>(receive_address);

  host_command->bus_data.command_code = device_request::command_code;
  host_command->bus_data.destination_ap = ap::de::device | ap::port_select::a;
  host_command->bus_data.source_ap = ap::port_select::a;
  host_command->bus_data.data_size = 0;
}

void maple_dma_start(uint32_t * command_buf)
{
  sh7091.DMAC.DMAOR = DMAOR__DDT                 /* on-demand data transfer mode */
                    | DMAOR__PR__CH2_CH0_CH1_CH3 /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | DMAOR__DME;                /* DMAC master enable */

  // clear maple-DMA end status
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;

  // disable maple-DMA
  maple_if.MDEN = mden::dma_enable::abort;

  volatile uint32_t _dummy = maple_if.MDST;
  (void)_dummy;

  // 20nsec * 0xc350 = 1ms
  constexpr uint32_t one_msec = 0xc350;
  maple_if.MSYS = msys::time_out_counter(one_msec)
                | msys::sending_rate::_2M;

  maple_if.MDTSEL = mdtsel::trigger_select::software_initiation;

  /* top address: the first/lowest address
     bottom address: the last/highest address */
  maple_if.MDAPRO = mdapro::security_code
		  | mdapro::top_address(0x00)
		  | mdapro::bottom_address(0x7f);

  maple_if.MDSTAR = mdstar::table_address(reinterpret_cast<uint32_t>(command_buf));

  maple_if.MDEN = mden::dma_enable::enable;
  maple_if.MDST = mdst::start_status::start;

  // wait for completion
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_MAPLE_DMA) == 0);

  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;
}
