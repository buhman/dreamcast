#include <cstdint>
#include <type_traits>

#include "sh7091.hpp"
#include "sh7091_bits.hpp"

#include "string.hpp"
#include "serial.hpp"

namespace serial {

void init_wait()
{
  sh7091.TMU.TSTR &= (~tmu::tstr::str1::counter_start) & 0xff; // stop TCNT1
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR1 = tmu::tcr1::tpsc::p_phi_1024; // 1024 / 50MHz = 20.48 μs
  sh7091.TMU.TCOR1 = 0xffff'ffff;
  sh7091.TMU.TCNT1 = 0xffff'ffff;
  sh7091.TMU.TSTR |= tmu::tstr::str1::counter_start;

  uint32_t start = sh7091.TMU.TCNT1;
  while ((start - sh7091.TMU.TCNT1) < 20);

  sh7091.TMU.TSTR &= (~tmu::tstr::str1::counter_start) & 0xff; // stop TCNT1
}

void reset_txrx()
{
  using namespace scif;

  sh7091.SCIF.SCFCR2 |= ( scfcr2::tfrst::reset_operation_enabled
			| scfcr2::rfrst::reset_operation_enabled
			);

  sh7091.SCIF.SCFCR2 &= ~( scfcr2::tfrst::reset_operation_enabled
			 | scfcr2::rfrst::reset_operation_enabled
			 );
}

void init(uint8_t bit_rate)
{
  using namespace scif;

  sh7091.SCIF.SCSCR2 = 0; // disable transmission / reception

  sh7091.SCIF.SCSPTR2 = 0; // clear output data pins

  sh7091.SCIF.SCFCR2 = scfcr2::tfrst::reset_operation_enabled
                     | scfcr2::rfrst::reset_operation_enabled;

  sh7091.SCIF.SCSMR2 = scsmr2::chr::_8_bit_data
		     | scsmr2::pe::parity_disabled
		     | scsmr2::stop::_1_stop_bit
		     | scsmr2::cks::p_phi_clock;

  sh7091.SCIF.SCBRR2 = bit_rate; // bps = 1562500 / (SCBRR2 + 1)

  sh7091.SCIF.SCFSR2 = (~scfsr2::er::bit_mask)
		     & (~scfsr2::tend::bit_mask)
		     & (~scfsr2::tdfe::bit_mask)
		     & (~scfsr2::brk::bit_mask)
		     & (~scfsr2::rdf::bit_mask)
		     & (~scfsr2::dr::bit_mask)
		     & 0xffff;


  // wait 1 bit interval
  init_wait();

  sh7091.SCIF.SCFCR2 = scfcr2::rtrg::trigger_on_1_byte
		     | scfcr2::ttrg::trigger_on_8_bytes
		     //| scfcr2::mce::modem_signals_enabled
		     ;

  sh7091.SCIF.SCSCR2 = scscr2::te::transmission_enabled
                     | scscr2::re::reception_enabled;

  sh7091.SCIF.SCLSR2 = 0; // clear ORER
}

void character(const char c)
{
  using namespace scif;
  // wait for transmit fifo to become partially empty
  while ((sh7091.SCIF.SCFSR2 & scfsr2::tdfe::bit_mask) == 0);

  sh7091.SCIF.SCFSR2 &= ~scfsr2::tdfe::bit_mask;

  sh7091.SCIF.SCFTDR2 = static_cast<uint8_t>(c);
}

void string(const char * s)
{
  while (*s != '\0') {
    character(*s++);
  }
}

void string(const uint8_t * s, uint32_t len)
{
  while (len > 0) {
    //hexlify(*s++);
    //character(' ');
    character(*s++);
    len--;
  }
}

void hexlify(const uint8_t n)
{
  constexpr uint32_t length = 2;
  char num_buf[length];
  string::hex<char>(num_buf, length, n);
  character(num_buf[0]);
  character(num_buf[1]);
}

void hexlify(const uint8_t * s, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    hexlify(s[i]);
    character(' ');
  }
  character('\n');
}

void hexlify(const uint32_t * s, uint32_t len)
{
  hexlify(reinterpret_cast<const uint8_t *>(s), len);
}

template <typename T, typename conv_type>
void integer(const T n, const char end, const uint32_t length)
{
  uint8_t num_buf[length];
  conv_type::template render<uint8_t>(num_buf, length, n);
  if constexpr (std::is_same<conv_type, string::hex_type>::value)
    string("0x");
  string(num_buf, length);
  character(end);
}

template void integer<uint32_t, hex>(uint32_t param, char end, uint32_t length);
template void integer<uint16_t, hex>(uint16_t param, char end, uint32_t length);
template void integer<uint8_t, hex>(uint8_t param, char end, uint32_t length);

template void integer<uint32_t, dec>(uint32_t param, char end, uint32_t length);
template void integer<uint16_t, dec>(uint16_t param, char end, uint32_t length);
template void integer<uint8_t, dec>(uint8_t param, char end, uint32_t length);

}
