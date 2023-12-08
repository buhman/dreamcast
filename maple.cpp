#define AP__PO__A (0b00 << 6)
#define AP__PO__B (0b01 << 6)
#define AP__PO__C (0b10 << 6)
#define AP__PO__D (0b11 << 6)

#define AP__DE__DEVICE = (1 << 5)
#define AP__DE__EXPANSION_DEVICE = (0 << 5)
#define AP__DE__PORT = (0 << 5)

#define AP__LM(reg) ((reg) & 0b11111)

// 2.6.8 "Peripheral Data Transfers"
// 5 "User Interface"; page 269

#define HOST_INSTRUCTION__END_FLAG (1 << 31)
#define HOST_INSTRUCTION__PORT_SELECT__A (0b00 << 16)
#define HOST_INSTRUCTION__PORT_SELECT__B (0b01 << 16)
#define HOST_INSTRUCTION__PORT_SELECT__C (0b10 << 16)
#define HOST_INSTRUCTION__PORT_SELECT__D (0b11 << 16)
#define HOST_INSTRUCTION__TRANSFER_LENGTH(n) (((n) & 0xff) << 0)

template <int N>
struct maple_host_command {
  uint32_t host_instruction;
  uint32_t receive_data_storage_address;
  uint32_t protocol_data[N];
};

void maple_host_command(uint32_t * buf, uint32_t * receive_address)
{
  auto command = reinterpet_cast<maple_host_command<1> *>(buf);

  command->host_instruction = HOST_INSTRUCTION__END_FLAG
			    | HOST_INSTRUCTION__PORT_SELECT__A
			    | HOST_INSTRUCTION__TRANSFER_LENGTH(0); // 4 bytes

  command->receive_data_storage_address = reinterpret_cast<uint32_t>(receive_address);

  uint32_t command_code = 0x01; // 'Device Request'
  uint32_t destination_ap = AP__DE__DEVICE | AP__PO__A;
  uint32_t source_ap = AP__PO__A;
  uint32_t data_size = 0;
  command->protocol_data[0] = (command_code << 24)
			    | (destination_ap << 16)
			    | (source_ap << 8)
                            | (data_size << 0);
}

void maple_dma_start(uint32_t * command_buf)
{
  sh7091.DMAC.DMAOR = DMAOR__DDT                 /* on-demand data transfer mode */
                    | DMAOR__PR__CH2_CH0_CH1_CH3 /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | DMAOR__DME;                /* DMAC master enable */

  // clear maple-DMA end status
  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;

  // disable maple-DMA
  system.MDEN = mden::dma_enable::abort;

  volatile uint32_t _dummy = system.MDST;
  (void)_dummy;

  // 20nsec * 0xc350 = 1ms
  constexpr uint32_t one_msec = 0xc350;
  system.MSYS = msys::time_out_counter(one_msec)
              | msys::sending_rate::_2M;

  system.MDTSEL = mdtsel::trigger_select::software_initiation;

  /* top address: the first/lowest address
     bottom address: the last/highest address */
  system.MDAPRO = mdapro::security_code
		| mdapro::top_address(0x00)
		| mdapro::bottom_address(0x7f);

  system.MDSTAR = mdstar::table_address(command_buf);

  system.MDEN = mden::dma_enable::enable;
  system.MDST = mdst::start_status::start;

  // wait for completion
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_MAPLE_DMA) == 0);

  system.ISTNRM = ISTNRM__END_OF_DMA_MAPLE_DMA;
}
