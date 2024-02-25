#include <cstdint>

#include "sh7091.hpp"
#include "sh7091_bits.hpp"

#include "string.hpp"

namespace serial {

void init(uint8_t bit_rate)
{
  using namespace scif;

  sh7091.SCIF.SCSCR2 = 0;
  sh7091.SCIF.SCSMR2 = 0;
  sh7091.SCIF.SCBRR2 = bit_rate; // bps = 1562500 / (SCBRR2 + 1)

  sh7091.SCIF.SCFCR2 = scfcr2::tfrst::reset_operation_enabled
                     | scfcr2::rfrst::reset_operation_enabled;
  // tx/rx trigger on 1 byte
  sh7091.SCIF.SCFCR2 = 0;

  sh7091.SCIF.SCSPTR2 = 0;
  sh7091.SCIF.SCLSR2 = 0;

  sh7091.SCIF.SCSCR2 = scscr2::te::transmission_enabled
                     | scscr2::re::reception_enabled;
}

void character(const char c)
{
  using namespace scif;
  // wait for transmit fifo to become empty
  while ((sh7091.SCIF.SCFSR2 & scfsr2::tdfe::bit_mask) == 0);

  for (int i = 0; i < 10000; i++) {
    asm volatile ("nop;");
  }

  sh7091.SCIF.SCFTDR2 = static_cast<uint8_t>(c);
}

void string(const char * s)
{
  while (*s != '\0') {
    character(*s++);
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

template <typename T>
void integer(const T n, const char end)
{
  constexpr uint32_t length = (sizeof (T)) * 2;
  char num_buf[length + 1];
  string::hex<char>(num_buf, length, n);
  num_buf[length] = 0;
  string("0x");
  string(num_buf);
  character(end);
}

template <typename T>
void integer(const T n)
{
  return integer(n, '\n');
}

template void integer<uint32_t>(uint32_t param);
template void integer<uint16_t>(uint16_t param);
template void integer<uint8_t>(uint8_t param);

template void integer<uint32_t>(uint32_t param, char end);
template void integer<uint16_t>(uint16_t param, char end);
template void integer<uint8_t>(uint8_t param, char end);

}
