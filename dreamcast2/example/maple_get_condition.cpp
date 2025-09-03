#include "systembus/systembus.hpp"
#include "systembus/systembus_bits.hpp"

#include "maple/maple_bits.hpp"
#include "maple/maple_host.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "maple/maple_bus_ft6.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

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

constexpr inline uint32_t align_32byte(uint32_t mem)
{
  return (mem + 31) & ~31;
}

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
void maple_dma_start(void * command_buf, uint32_t send_size,
                     void * response_buf, uint32_t recv_size)
{
  // write back operand cache blocks for command buffer prior to starting DMA
  for (uint32_t i = 0; i < align_32byte(send_size) / 32; i++) {
    asm volatile ("ocbwb @%0"
                  :
                  : "r" (reinterpret_cast<uint32_t>(((uint32_t)command_buf) + 32 * i))
                  );
  }

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

  // write back operand cache blocks for command buffer prior to starting DMA
  for (uint32_t i = 0; i < align_32byte(recv_size) / 32; i++) {
    asm volatile ("ocbi @%0"
                  :
                  : "r" (reinterpret_cast<uint32_t>(((uint32_t)response_buf) + 32 * i))
                  );
  }
}

int maple_dma_wait_complete()
{
  using systembus::systembus;
  using namespace systembus;

  // wait for Maple DMA completion
  while ((systembus.ISTNRM & istnrm::end_of_dma_maple_dma) == 0) {
    if (systembus.ISTERR) {
      string("ISTERR ");
      print_base16(systembus.ISTERR, 8);
      character('\n');
      if ((systembus.ISTERR >> 11) & 1) {
        return -1;
      }
    }
  }
  // clear Maple DMA end status
  systembus.ISTNRM = istnrm::end_of_dma_maple_dma;

  return 0;
}

template <typename C, typename R>
void maple_device_request(maple::host_command<typename C::data_fields> * host_command,
                          maple::host_response<typename R::data_fields> * host_response,
                          bool end_flag = false)
{
  using namespace maple;

  constexpr uint32_t data_size = (sizeof (typename C::data_fields));

  host_command->host_instruction = (end_flag ? host_instruction::end_flag : 0)
                                 | host_instruction::port_select::a
                                 | host_instruction::pattern::normal
                                 | host_instruction::transfer_length(data_size / 4);

  host_command->receive_data_address = ((uint32_t)host_response) & receive_data_address::mask;

  host_command->header.command_code = C::command_code;

  host_command->header.destination_ap = ap::port_select::a
                                      | ap::de::device;

  host_command->header.source_ap = ap::port_select::a
                                 | ap::de::port;

  host_command->header.data_size = data_size / 4;
}

// the Maple bus protocol is big endian, but the SH4 is running in little endian mode
#define bswap32 __builtin_bswap32
#define bswap16 __builtin_bswap16

void main()
{
  // Maple DMA buffers must be 32-byte aligned
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  struct requests {
    maple::host_command<maple::device_request::data_fields> device_request;
    maple::host_command<maple::get_condition::data_fields> get_condition;
  };
  static_assert((sizeof (requests)) == (sizeof (maple::host_command<maple::device_request::data_fields>)) + (sizeof (maple::host_command<maple::get_condition::data_fields>)));

  using maple__data_transfer = maple::data_transfer<maple::ft0::data_transfer::data_format>;

  struct responses {
    maple::host_response<maple::device_status::data_fields> device_status;
    maple::host_response<maple__data_transfer::data_fields> data_transfer;
  };

  auto requests = (struct requests *)send_buf;
  auto responses = (struct responses *)recv_buf;

  // fill send_buf with a "device request" command
  // recv_buf is reply destination address
  maple_device_request<maple::device_request, maple::device_status>(&requests->device_request, &responses->device_status);
  maple_device_request<maple::get_condition, maple__data_transfer>(&requests->get_condition, &responses->data_transfer, true);

  requests->get_condition.data.function_type = bswap32(maple::function_type::controller);

  systembus::systembus.ISTERR = 0xffffffff;

  maple_dma_start(send_buf, (sizeof (struct requests)),
                  recv_buf, (sizeof (struct responses)));

  string("wait\n");
  int res = maple_dma_wait_complete();
  if (res != 0) {
    string("   ");
    return;
  }

  if (responses->device_status.header.command_code != maple::device_status::command_code) {
    string("maple port A: invalid response or disconnected\n");
    string("   ");
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  // data transfer
  //////////////////////////////////////////////////////////////////////////////

  string("responses->device_status:\n");
  string("  header:\n");
  string("    command_code   : ");
  print_base16(responses->device_status.header.command_code, 2); character('\n');
  string("    destination_ap : ");
  print_base16(responses->device_status.header.destination_ap, 2); character('\n');
  string("    source_ap      : ");
  print_base16(responses->device_status.header.source_ap, 2); character('\n');
  string("    data_size      : ");
  print_base16(responses->device_status.header.data_size, 2); character('\n');

  auto& device_status = responses->device_status.data;

  string("  data:\n");
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

  //////////////////////////////////////////////////////////////////////////////
  // data transfer
  //////////////////////////////////////////////////////////////////////////////

  if (responses->data_transfer.header.command_code != maple__data_transfer::command_code) {
    string("maple port A: invalid response or disconnected\n");
    print_base16(responses->data_transfer.header.command_code, 8);
    character('\n');
    string("   ");
    return;
  }

  string("responses->data_transfer:\n");
  string("  header:\n");
  string("    command_code   : ");
  print_base16(responses->data_transfer.header.command_code, 2); character('\n');
  string("    destination_ap : ");
  print_base16(responses->data_transfer.header.destination_ap, 2); character('\n');
  string("    source_ap      : ");
  print_base16(responses->data_transfer.header.source_ap, 2); character('\n');
  string("    data_size      : ");
  print_base16(responses->data_transfer.header.data_size, 2); character('\n');

  auto& data_transfer = responses->data_transfer.data;

  string("  data:\n");
  string("    function_type: ");
  print_base16(bswap32(data_transfer.function_type), 8); character('\n');
  string("    data:\n");
  string("      digital_button: ");
  print_base16(bswap16(data_transfer.data.digital_button), 4); character('\n');
  string("      analog_coordinate_axis[0]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[0], 2); character('\n');
  string("      analog_coordinate_axis[1]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[1], 2); character('\n');
  string("      analog_coordinate_axis[2]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[2], 2); character('\n');
  string("      analog_coordinate_axis[3]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[3], 2); character('\n');
  string("      analog_coordinate_axis[4]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[4], 2); character('\n');
  string("      analog_coordinate_axis[5]: ");
  print_base16(data_transfer.data.analog_coordinate_axis[5], 2); character('\n');

  string("   ");
}
