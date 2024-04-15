#include <cstdint>

#include "sh7091/serial.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/vbr.hpp"
#include "systembus.hpp"

void vbr100()
{
  serial::string("vbr100\n");
  serial::string("expevt ");
  serial::integer(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer(sh7091.CCN.INTEVT);
  uint32_t spc;
  uint32_t ssr;
  asm volatile ("stc spc,%0"
		: "=r" (spc)
		);
  asm volatile ("stc ssr,%0"
		: "=r" (ssr)
		);

  serial::string("spc ");
  serial::integer(spc);
  serial::string("ssr ");
  serial::integer(ssr);
  while (1);
}

void vbr400()
{
  serial::string("vbr400");
  while (1);
}

void vbr600()
{
  serial::string("vbr600");
  while (1);
}

extern "C" uint32_t * illslot(void);
void main()
{
  uint32_t vbr = reinterpret_cast<uint32_t>(&__vbr_link_start) - 0x100;

  system.IML2NRM = 0;
  system.IML2ERR = 0;
  system.IML2EXT = 0;

  system.IML4NRM = 0;
  system.IML4ERR = 0;
  system.IML4EXT = 0;

  system.IML6NRM = 0;
  system.IML6ERR = 0;
  system.IML6EXT = 0;

  sh7091.CCN.INTEVT = 0;
  sh7091.CCN.EXPEVT = 0;

  uint32_t zero = 0;
  asm volatile ("ldc %0,spc"
		:
		: "r" (zero));

  asm volatile ("ldc %0,ssr"
		:
		: "r" (zero));

  asm volatile ("ldc %0,vbr"
		:
		: "r" (vbr));


  uint32_t sr;
  asm volatile ("stc sr,%0"
		: "=r" (sr));

  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  sr = sr & (~(1 << 28)); // BL

  asm volatile ("ldc %0, sr"
		:
		: "r" (sr));

  /*
  uint32_t vbr2;
  asm volatile ("stc vbr,%0"
		: "=r" (vbr2));
  */

  serial::integer<uint32_t>(vbr);
  //serial::integer<uint32_t>(vbr2);
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&vbr100));

  uint32_t * test = illslot();
  serial::integer<uint32_t>(*test);

  while (1);
}
