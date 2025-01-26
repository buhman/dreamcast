#include "stdint.h"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/vbr.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"

extern "C" float fipr(float * a, float * b);
extern "C" void sobel_fipr(float * a, int * i);

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
  serial::string("vbr400\n");
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

void vbr600()
{
  serial::string("vbr600\n");
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

void test1()
{
  float a[] = {1, 2, 3, 4};
  float b[] = {5, 6, 7, 8};

  // 70

  union {
    float f;
    uint32_t i;
  } v;

  v.f = fipr(a, b);
  serial::integer(v.i);
}

void test2()
{
  float a[640 * 480];
  a[0] = 11;
  a[1] = 12;
  a[2] = 13;
  a[0 + 640] = 1400;
  a[1 + 640] = 1500;
  a[2 + 640] = 1600;
  a[0 + 1280] = 170000;
  a[1 + 1280] = 180000;
  a[2 + 1280] = 190000;

  // -719952
  // -20402
  // 518747123908

  int i[640 * 480];

  // expected value:
  for (int j = 0; j < 640 * 480; j++) {
    i[j] = 0xeeeeeeee;
  }

  sobel_fipr(a, i);
  // -5952

  int v;
  v = i[640 + 1];
  serial::integer<uint32_t>(v);
  v = i[640 + 2];
  serial::integer<uint32_t>(v);
  v = i[640 + 3];

  v = i[640 * 479 - 1];
  serial::integer<uint32_t>(v);
  v = i[640 * 479 - 2];
  serial::integer<uint32_t>(v);
  v = i[640 * 479 - 3];
  serial::integer<uint32_t>(v);
  v = i[640 * 479 - 4];
  serial::integer<uint32_t>(v);
  v = i[640 * 479 - 5];
  serial::integer<uint32_t>(v);
  v = i[640 * 479 - 6];
  serial::integer<uint32_t>(v);
}

void init_interrupt()
{
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

  uint32_t vbr = reinterpret_cast<uint32_t>(&__vbr_link_start) - 0x100;

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
}

void main()
{
  init_interrupt();

  serial::string("test1:\n");
  test1();
  serial::string("test2:\n");
  serial::string("test2:\n");
  serial::string("test2:\n");
  test2();

  serial::string("return\n");
  serial::string("return\n");
  serial::string("return\n");
}
