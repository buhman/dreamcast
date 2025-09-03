#include "systembus/systembus.hpp"
#include "systembus/systembus_bits.hpp"

#include "maple/maple_bits.hpp"
#include "maple/maple_host.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

/*
  See DCDBSysArc990907E.pdf page 274.

  Maple DMA uses SH4 DMA channel 0 in "DDT" mode. This means that Holly's Maple
  DMA functional unit is externally controlling the SH4's DMA unit.

  The Dreamcast boot rom leaves channel 0 already configured for Maple DMA,
  though if you felt rebellious and wished to used channel 0 for something else,
  you would need to reconfigure channel 0 for Maple/DDT afterwards.

  Note that this `maple_dma_start` function does not configure SH4 DMA channel
  0, and presumes it is already in the correct state.
 */
void maple_dma_start(void * command_buf)
{
  using systembus::maple_if;
  using systembus::systembus;
  using namespace maple;
  using namespace systembus;

  // if SH4 cache were enabled, it would be necessary to use the `ocbwb` sh4
  // instruction to write back the cache to system memory, prior to continuing.

  // if SH4 cache were enabled, it would be necessary to use the `ocbi` sh4
  // instruction to invalidate any possibly-cached areas of the receive buffer,
  // as these are imminently going to be rewritten by the DMA unit independently
  // of cache access.

  // disable Maple DMA and abort any possibly-in-progress transfers
  maple_if.MDEN = mden::dma_enable::abort;
  while ((maple_if.MDST & mdst::start_status::bit_mask) != 0);

  // clear Maple DMA end status
  systembus.ISTNRM = istnrm::end_of_dma_maple_dma;

  // 20nsec maple_if. 0xc350 = 1ms
  uint32_t one_msec = 0xc350;
  // set the Maple bus controller timeout and transfer rate
  maple_if.MSYS = msys::time_out_counter(one_msec)
                | msys::sending_rate::_2M;

  // MDAPRO controls which (system memory) addresses are considered valid for
  // Maple DMA. An attempt to use an address outside of this range will cause an
  // error interrupt in from the Maple DMA unit.
  //
  // 0x40 through 0x7F allows for transfers from 0x0c000000 to 0x0fffffff
  // System Memory is 0x0c000000 through 0x0cffffff (16MB)
  //
  // It is probably possible to do strange things such as use texture memory as
  // a Maple DMA receive buffer, but TOP_ADDRESS would need to be lowered
  // accordingly.
  maple_if.MDAPRO = mdapro::security_code
                  | mdapro::top_address(0x40)
                  | mdapro::bottom_address(0x7f);

  // SOFTWARE_INITIATION allows a Maple DMA transfer to be initiated by a write
  // to the MDST register.
  maple_if.MDTSEL = mdtsel::trigger_select::software_initiation;

  // the Maple DMA start address must be 32-byte aligned
  maple_if.MDSTAR = mdstar::table_address((uint32_t)command_buf);

  // re-enable Maple DMA
  maple_if.MDEN = mden::dma_enable::enable;

  // start Maple DMA (completes asynchronously)
  maple_if.MDST = mdst::start_status::start;
}

void maple_dma_wait_complete()
{
  using systembus::systembus;
  using namespace systembus;

  // wait for Maple DMA completion
  while ((systembus.ISTNRM & istnrm::end_of_dma_maple_dma) == 0);
  // clear Maple DMA end status
  systembus.ISTNRM = istnrm::end_of_dma_maple_dma;
}

void maple_device_request(uint8_t * send_buf, uint8_t * recv_buf)
{
  using namespace maple;

  auto host_command = (maple::host_command<device_request::data_fields> *)(send_buf);

  host_command->host_instruction = host_instruction::end_flag
                                 | host_instruction::port_select::a
                                 | host_instruction::pattern::normal
                                 | host_instruction::transfer_length(0 / 4);

  host_command->receive_data_address = ((uint32_t)recv_buf) & receive_data_address::mask;

  host_command->header.command_code = device_request::command_code;

  host_command->header.destination_ap = ap::port_select::a
                                      | ap::de::device;

  host_command->header.source_ap = ap::port_select::a
                                 | ap::de::port;

  host_command->header.data_size = 0 / 4;
}

static inline void character(const char c)
{
  using sh7091::sh7091;
  using namespace sh7091;

  // set the transmit trigger to `1 byte`--this changes the behavior of TDFE
  sh7091.SCIF.SCFCR2 = scif::scfcr2::ttrg::trigger_on_1_bytes;

  // wait for transmit fifo to become partially empty
  while ((sh7091.SCIF.SCFSR2 & scif::scfsr2::tdfe::bit_mask) == 0);

  // unset tdfe bit
  sh7091.SCIF.SCFSR2 = (uint16_t)(~scif::scfsr2::tdfe::bit_mask);

  sh7091.SCIF.SCFTDR2 = c;
}

static void string(const char * s)
{
  while (*s != 0) {
    character(*s++);
  }
}

static void print_base16(uint32_t n, int len)
{
  char buf[len];
  char * bufi = &buf[len - 1];

  while (bufi >= buf) {
    uint32_t nib = n & 0xf;
    n = n >> 4;
    if (nib > 9) {
      nib += (97 - 10);
    } else {
      nib += (48 - 0);
    }

    *bufi = nib;
    bufi -= 1;
  }

  for (int i = 0; i < len; i++) {
    character(buf[i]);
  }
}

void main()
{
  // Maple DMA buffers must be 32-byte aligned
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  // fill send_buf with a "device request" command
  // recv_buf is reply destination address
  maple_device_request(send_buf, recv_buf);

  maple_dma_start(send_buf);

  maple_dma_wait_complete();

  // decode the reply in recv_buf
  auto host_response = (maple::host_response<maple::device_status::data_fields> *)recv_buf;

  if (host_response->header.command_code != maple::device_status::command_code) {
    string("maple port A: invalid response or disconnected\n");
    return;
  }

  string("host_response:\n");
  string("  protocol_header:\n");
  string("    command_code   : ");
  print_base16(host_response->header.command_code, 2); character('\n');
  string("    destination_ap : ");
  print_base16(host_response->header.destination_ap, 2); character('\n');
  string("    source_ap      : ");
  print_base16(host_response->header.source_ap, 2); character('\n');
  string("    data_size      : ");
  print_base16(host_response->header.data_size, 2); character('\n');

  // the Maple bus protocol is big endian, but the SH4 is running in little endian mode
  #define bswap32 __builtin_bswap32
  #define bswap16 __builtin_bswap16

  auto& device_status = host_response->data;

  string("  device_status:\n");
  string("    device_id:\n");
  string("      ft   : ");
  print_base16(bswap32(device_status.device_id.ft), 8); character('\n');
  string("      fd[0]: ");
  print_base16(bswap32(device_status.device_id.fd[0]), 8); character('\n');
  string("      fd[1]: ");
  print_base16(bswap32(device_status.device_id.fd[1]), 8); character('\n');
  string("      fd[2]: ");
  print_base16(bswap32(device_status.device_id.fd[2]), 8); character('\n');
  string("    destination_code: ");
  print_base16(bswap32(device_status.destination_code), 2); character('\n');
  string("    connection_direction: ");
  print_base16(bswap32(device_status.connection_direction), 2); character('\n');
  string("    product_name: \"");
  for (int i = 0; i < 30; i++)
    character(device_status.product_name[i]);
  string("\"\n");
  string("    license: \"");
  for (int i = 0; i < 60; i++)
    character(device_status.license[i]);
  string("\"\n");
  string("    low_consumption_standby_current: ");
  print_base16(bswap16(device_status.low_consumption_standby_current), 4); character('\n');
  string("    maximum_current_consumption: ");
  print_base16(bswap16(device_status.maximum_current_consumption), 4); character('\n');

  string("   ");
}
