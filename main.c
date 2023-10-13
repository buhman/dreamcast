#include <stdint.h>

#include "cache.h"

#include "sh7091.h"
#include "sh7091_bits.h"
#include "holly.h"

volatile uint32_t * RAM = (volatile uint32_t *)0xa5000000;

volatile uint32_t * SPG_STATUS = (volatile uint32_t *)0xa05f810;

uint32_t get_cable_type()
{
  /* set all pins to input */
  SH7091.BSC.PCTRA = 0;

  /* get cable type from pins 9 + 8 */
  return SH7091.BSC.PDTRA & PDTRA__MASK;
}

void vga1()
{
  uint16_t pclk_delay = 0x0016;
  uint32_t fb_r_ctrl = HOLLY.FB_R_CTRL;
  HOLLY.FB_R_CTRL = fb_r_ctrl & ~(1 << 0); // fb_enable = 0
  HOLLY.VO_CONTROL = pclk_delay << 16 | (1 << 3); // blank_video
  HOLLY.FB_R_CTRL = fb_r_ctrl & (1<<23); // vclk_div, for VGA

  HOLLY.FB_R_SIZE = 0;

  uint16_t vblank_in = 0x0208;
  uint16_t vblank_out = 0x0015;
  HOLLY.SPG_VBLANK_INT = (vblank_out << 16) | (vblank_in << 0);

  HOLLY.SPG_CONTROL = (1 << 8); // sync_direction__output ; non-default

  uint16_t hbstart = 0x0345; // default
  uint16_t hbend   = 0x007e; // default
  HOLLY.SPG_HBLANK = (hbend << 16) | (hbstart << 0);

  uint16_t hcount = 0x0359; // default
  uint16_t vcount = 0x020c; // non-default
  HOLLY.SPG_LOAD = (vcount << 16) | (hcount << 0);

  uint16_t vbstart = 0x0208; // non-default
  uint16_t vbend = 0x0028; // non-default
  HOLLY.SPG_VBLANK = (vbend << 16) | (vbstart << 0);

  uint16_t hswidth = 0x003f;
  uint16_t vswidth = 0x0003;
  uint16_t bpwidth = 0x0319;
  uint16_t eqwidth = 0x000f;
  HOLLY.SPG_WIDTH =
    (hswidth << 0)
    | (vswidth << 8)
    | (bpwidth << 12)
    | (eqwidth << 22);

  uint16_t startx = 0x0a8;
  uint16_t starty = 0x028;
  HOLLY.VO_STARTX = startx;
  HOLLY.VO_STARTY = (starty << 16) | (starty << 0);

  HOLLY.SPG_HBLANK_INT = hbstart << 16;
}

void vga2()
{
  uint16_t xsize = 640;
  uint16_t ysize = 480;

  uint16_t fb_xclip_min = 0;
  uint16_t fb_xclip_max = xsize-1;
  HOLLY.FB_X_CLIP = (fb_xclip_max << 16) | (fb_xclip_min << 0);
  uint16_t fb_yclip_min = 0;
  uint16_t fb_yclip_max = ysize-1;
  HOLLY.FB_Y_CLIP = (fb_yclip_max << 16) | (fb_yclip_min << 0);

  uint16_t fb_latency = 0x09;
  uint16_t fb_burstctrl = 15 - fb_latency;
  uint16_t wr_burst = 0x08;
  HOLLY.FB_BURSTCTRL = 0
    | (fb_burstctrl << 0)
    | (fb_latency << 4)
    | (wr_burst << 9)
    ;

  uint32_t fb_xsize = (xsize * 16)/(32) - 1;
  uint32_t fb_ysize = ysize - 3;
  uint32_t fb_mod = 1;
  HOLLY.FB_R_SIZE = 0
    | (fb_xsize << 0)
    | (fb_ysize << 10)
    | (fb_mod << 20);

  uint16_t coeff0 = 0x40;
  uint16_t coeff1 = 0x80;
  HOLLY.Y_COEFF = (coeff1 << 8) | (coeff0 << 0);

  uint16_t vscale_factor = 0x0400;
  HOLLY.SCALER_CTL = (vscale_factor << 0);

  uint32_t fb_linestride = (xsize * 16) / 64;
  HOLLY.FB_W_LINESTRIDE = fb_linestride;

  HOLLY.FB_W_CTRL = 0
    | 0b001 << 0 // fb_packmode: RGB 565
    ;

  HOLLY.FB_W_SOF1 = 0;
  HOLLY.FB_W_SOF2 = 0;
  HOLLY.FB_R_SOF1 = 0;
  HOLLY.FB_R_SOF2 = 0;

  HOLLY.FB_R_CTRL = 0
    | 1 << 23 // vclk_div
    | 0 << 22 // fb_strip_buf_en
    | 0 << 16 // fb_strip_size
    | 0 << 8 // fb_chroma_threshold
    | 0 << 4 // fb_concat
    | 1 << 2 // fb_depth
    | 0 << 1 // fb_line_double
    | 1 << 0 // fb_enable
    ;

  *((reg32 *)0xa0702c00) = 0;
}

void v_sync_in()
{
#define V_SYNC (1<<13)
  while (!(V_SYNC & HOLLY.SPG_STATUS)) {
    asm volatile ("nop");
  }
  while ((V_SYNC & HOLLY.SPG_STATUS)) {
    asm volatile ("nop");
  }
#undef V_SYNC
}

void vga()
{
  get_cable_type();

  HOLLY.SOFTRESET = 0;
  HOLLY.TEXT_CONTROL = 3;
  HOLLY.FB_W_CTRL = 9;

  SH7091.SCIF.SCFTDR2 = 'g';
  /*
   */
  SH7091.SCIF.SCFTDR2 = 'v';
  vga1();
  vga2();

  v_sync_in();

  HOLLY.VO_BORDER_COL = 31;
  HOLLY.VO_CONTROL = 0x0016;
}

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

struct rgb
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct hsv
{
  unsigned char h;
  unsigned char s;
  unsigned char v;
};

struct rgb hsv_to_rgb(struct hsv hsv)
{
  struct rgb rgb;
  unsigned char region, remainder, p, q, t;

  if (hsv.s == 0) {
    rgb.r = hsv.v;
    rgb.g = hsv.v;
    rgb.b = hsv.v;
    return rgb;
  }

  region = hsv.h / 43;
  remainder = (hsv.h - (region * 43)) * 6;

  p = (hsv.v * (255 - hsv.s)) >> 8;
  q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
  t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:  rgb.r = hsv.v; rgb.g = t;     rgb.b = p;     break;
  case 1:  rgb.r = q;     rgb.g = hsv.v; rgb.b = p;     break;
  case 2:  rgb.r = p;     rgb.g = hsv.v; rgb.b = t;     break;
  case 3:  rgb.r = p;     rgb.g = q;     rgb.b = hsv.v; break;
  case 4:  rgb.r = t;     rgb.g = p;     rgb.b = hsv.v; break;
  default: rgb.r = hsv.v; rgb.g = p;     rgb.b = q;     break;
  }

  return rgb;
}

void main()
{
  cache_init();

  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  // clear BSS

#define SCSCR2__TE (1 << 5)
#define SCSCR2__RE (1 << 4)
  SH7091.SCIF.SCSCR2 = 0;
  SH7091.SCIF.SCSMR2 = 0;
  SH7091.SCIF.SCBRR2 = 12;

  SH7091.SCIF.SCFCR2 = (1 << 2) | (1 << 1); // tfrst rfrst
  SH7091.SCIF.SCFCR2 = 0;

  SH7091.SCIF.SCSPTR2 = 0;
  SH7091.SCIF.SCLSR2 = 0;

  SH7091.SCIF.SCSCR2 = SCSCR2__TE | SCSCR2__RE;

  SH7091.SCIF.SCFTDR2 = 'H';
  SH7091.SCIF.SCFTDR2 = 'e';
  SH7091.SCIF.SCFTDR2 = 'l';
  SH7091.SCIF.SCFTDR2 = 'o';

  vga();

  while (1) {
    v_sync_in();
    SH7091.SCIF.SCFTDR2 = 'v';
    SH7091.SCIF.SCFTDR2 = 'g';
    SH7091.SCIF.SCFTDR2 = 'a';

    reg16 * vram = (reg16 *)0xa5000000;
    for (int y = 0; y < 480; y++) {
      for (int x = 0; x < 640; x++) {
        struct hsv hsv = {(y * 255) / 480, 255, 255};
        struct rgb rgb = hsv_to_rgb(hsv);
        vram[y * 640 + x] = (rgb.r >> 3) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 11);
      }
    }
    vram[0] = 0xf000;
    vram[10] = 0xf0ff;
    vram[11] = 0xf0ab;
  }
}
