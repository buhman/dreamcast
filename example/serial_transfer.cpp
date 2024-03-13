#include <cstdint>

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"

#include "serial_load.hpp"

void main() __attribute__((section(".text.main")));

void main()
{
  //serial::init(12);
  load_init();

  while (1) {
    using namespace scif;

    while ((scfdr2::receive_data_bytes(sh7091.SCIF.SCFDR2)) > 0) {
      const uint8_t c = sh7091.SCIF.SCFRDR2;
      load_recv(c);
    }
  }
}
