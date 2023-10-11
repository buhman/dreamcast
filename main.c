#include <stdint.h>

volatile uint8_t * SCFTDR2 = (volatile uint8_t *)0xFFE8000C;

volatile uint32_t * SOFT_RESET  = (volatile uint32_t *)0xa05f8008;
volatile uint32_t * STARTRENDER = (volatile uint32_t *)0xa05f8014;
volatile uint32_t * VO_CONTROL = (volatile uint32_t *)0xa05f80e8;
volatile uint32_t * VO_BORDER_COL = (volatile uint32_t *)0xa05f8040;

volatile uint32_t * FB_R_SOF1 = (volatile uint32_t *)0xa05f8050;

volatile uint32_t * RAM = (volatile uint32_t *)0xa5000000;

volatile uint32_t * SPG_STATUS = (volatile uint32_t *)0xa05f810;

void start()
{
  *SCFTDR2 = 'H';
  *SCFTDR2 = 'e';
  *SCFTDR2 = 'l';
  *SCFTDR2 = 'l';
  *SCFTDR2 = '3';

  *SOFT_RESET = 0b111;
  *SOFT_RESET = 0b000;

  *VO_CONTROL |= 1 << 3;
  *VO_BORDER_COL = 31;

  for (int i = 0; i < (2*1024*1024 + 1024 * 2 + 512 + 4) / 4; i++) {
    RAM[i] = 0x1;
  }

  while (1) {
  }
}

/*
    {
        DM_640x480,
        640, 480,
	VID_INTERLACE, // flags
        CT_VGA, // cable type
        0, // pixel mode
        0x20C, 0x359, // scanlines, clocks per scanline
        0xAC, 0x28,   // bitmap x, bitmap y
        0x15, 0x104,  // first scanline interrupt, second scanline interrupt
        0x7E, 0x345,  // border x start, border x stop
        0x24, 0x204,  // border y start, border y stop
        0, 1,         // current framebuffer, number of framebuffers
        { 0, 0, 0, 0 } // offset to framebuffers
    },
*/
