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
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);
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

__attribute__ ((interrupt_handler))
void dbr();

void dbr()
{
  serial::string("dbr\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);

  uint32_t spc;
  uint32_t ssr;
  uint32_t sgr;
  uint32_t r15;
  asm volatile ("stc spc,%0" : "=r" (spc) );
  asm volatile ("stc ssr,%0" : "=r" (ssr) );
  asm volatile ("stc sgr,%0" : "=r" (sgr) );
  asm volatile ("mov r15,%0" : "=r" (r15) );
  serial::string("spc ");
  serial::integer(spc);
  serial::string("ssr ");
  serial::integer(ssr);
  serial::string("sgr ");
  serial::integer(sgr);
  serial::string("r15 ");
  serial::integer(r15);

  uint32_t sr;
  asm volatile ("stc sr,%0" : "=r" (sr) );
  serial::string("sr ");
  serial::integer(sr);

  while (1);

  return;
}

int do_stuff(int a, int b)
{
  serial::string("do_stuff\n");
  asm volatile ("nop;");
  return a + b;
}

extern "C" uint32_t * illslot(void);

void main()
{
  serial::string("main\n");
  for (int i = 0; i < 10000000; i++) {
    asm volatile ("nop;");
  }
  //serial::init(0);

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

  sr &= ~sh::sr::bl; // BL
  sr |= sh::sr::imask(15); // imask

  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  asm volatile ("ldc %0,sr"
		:
		: "r" (sr));

  serial::string("vbr ");
  serial::integer<uint32_t>(vbr);
  serial::string("vbr100 ");
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&vbr100));

  (void)dbr;
  uint32_t dbr_address = reinterpret_cast<uint32_t>(&dbr);
  asm volatile ("ldc %0,dbr"
                :
                : "r" (dbr_address));
  serial::string("dbr ");
  serial::integer<uint32_t>(dbr_address);

  sh7091.UBC.BARA = reinterpret_cast<uint32_t>(&do_stuff);
  sh7091.CCN.BASRA = 0;
  sh7091.UBC.BAMRA
    = ubc::bamra::bama::all_bara_bits_are_included_in_break_conditions
    | ubc::bamra::basma::no_basra_bits_are_included_in_break_conditions
    ;
  sh7091.UBC.BBRA
    = ubc::bbra::sza::operand_size_is_not_included_in_break_conditions
    | ubc::bbra::ida::instruction_access_cycle_is_used_as_break_condition
    | ubc::bbra::rwa::read_cycle_or_write_cycle_is_used_as_break_condition
    ;
  sh7091.UBC.BRCR
    = ubc::brcr::pcba::channel_a_pc_break_is_effected_before_instruction_execution
    | ubc::brcr::ubde::user_break_debug_function_is_used
    ;
  serial::string("basra ");
  serial::integer(sh7091.CCN.BASRA);
  serial::string("bara ");
  serial::integer(sh7091.UBC.BARA);
  serial::string("bamra ");
  serial::integer(sh7091.UBC.BAMRA);
  serial::string("bbra ");
  serial::integer(sh7091.UBC.BBRA);
  serial::string("brcr ");
  serial::integer(sh7091.UBC.BRCR);

  uint32_t r15;
  asm volatile ("mov r15,%0" : "=r" (r15) );
  serial::string("r15 ");
  serial::integer(r15);
  int res = do_stuff(1, 2);
  (void)res;

  /*
  uint32_t * test = illslot();
  serial::string("illslot\n");
  serial::integer<uint32_t>(*test);
  */
  serial::string("return\n");

  //while (1);
}
