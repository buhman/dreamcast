#include <stdint.h>

#include "sh7091.h"
#include "sh7091_bits.h"
#include "holly.h"

#include "vga.h"
#include "rgb.h"

uint32_t get_cable_type()
{
  /* set all pins to input */
  SH7091.BSC.PCTRA = 0;

  /* get cable type from pins 9 + 8 */
  return SH7091.BSC.PDTRA & PDTRA__MASK;
}

void vga1()
{
  uint32_t fb_r_ctrl = HOLLY.FB_R_CTRL;
  HOLLY.FB_R_CTRL = fb_r_ctrl & ~(1 << 0); // fb_enable = 0
  uint32_t blank_video = 1;
  HOLLY.VO_CONTROL |= (blank_video << 3); // blank_video

  HOLLY.FB_R_SIZE = 0;

  uint32_t vblank_in = 0x0208;
  uint32_t vblank_out = 0x0015;
  HOLLY.SPG_VBLANK_INT = (vblank_out << 16) | (vblank_in << 0);

  uint32_t sync_direction = 1;
  HOLLY.SPG_CONTROL = (sync_direction << 8);

  uint32_t hbstart = 0x0345; // default
  uint32_t hbend   = 0x007e; // default
  HOLLY.SPG_HBLANK = (hbend << 16) | (hbstart << 0);

  uint32_t hcount = 0x0359; // default
  uint32_t vcount = 0x020c; // non-default
  HOLLY.SPG_LOAD = (vcount << 16) | (hcount << 0);

  uint32_t vbstart = 0x0208; // non-default
  uint32_t vbend = 0x0028; // non-default
  HOLLY.SPG_VBLANK = (vbend << 16) | (vbstart << 0);

  uint32_t hswidth = 0x003f;
  uint32_t vswidth = 0x0003;
  uint32_t bpwidth = 0x0319;
  uint32_t eqwidth = 0x000f;
  HOLLY.SPG_WIDTH =
    (hswidth << 0)
    | (vswidth << 8)
    | (bpwidth << 12)
    | (eqwidth << 22);

  uint32_t startx = 0x0a8;
  uint32_t starty = 0x028;
  HOLLY.VO_STARTX = startx;
  HOLLY.VO_STARTY = (starty << 16) | (starty << 0);

  HOLLY.SPG_HBLANK_INT = hbstart << 16;
}

void vga2()
{
  HOLLY.FB_BURSTCTRL = 0x00093f39;

  uint32_t xsize = 640;
  uint32_t ysize = 480;

  uint32_t fb_xclip_min = 0;
  uint32_t fb_xclip_max = xsize-1;
  HOLLY.FB_X_CLIP = (fb_xclip_max << 16) | (fb_xclip_min << 0);
  uint32_t fb_yclip_min = 0;
  uint32_t fb_yclip_max = ysize-1;
  HOLLY.FB_Y_CLIP = (fb_yclip_max << 16) | (fb_yclip_min << 0);

  uint32_t fb_xsize = (xsize * 16)/(32) - 1;
  uint32_t fb_ysize = ysize - 3;
  uint32_t fb_mod = 1;
  HOLLY.FB_R_SIZE = 0
    | (fb_xsize << 0)
    | (fb_ysize << 10)
    | (fb_mod << 20);

  uint32_t coeff0 = 0x40;
  uint32_t coeff1 = 0x80;
  HOLLY.Y_COEFF = (coeff1 << 8) | (coeff0 << 0);

  uint32_t vscale_factor = 0x0400;
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

  uint32_t hsync_pol    = 0;
  uint32_t vsync_pol    = 0;
  uint32_t blank_pol    = 0;
  uint32_t blank_video  = 0;
  uint32_t field_mode   = 0;
  uint32_t pixel_double = 0;
  uint32_t pclk_delay   = 0x16;
  HOLLY.VO_CONTROL = 0
    | (( pclk_delay   & 0x3f) << 16 )
    | (( pixel_double & 0x01) <<  8 )
    | (( field_mode   & 0x0f) <<  4 )
    | (( blank_video  & 0x01) <<  3 )
    | (( blank_pol    & 0x01) <<  2 )
    | (( vsync_pol    & 0x01) <<  1 )
    | (( hsync_pol    & 0x01) <<  0 );

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

void v_sync_out()
{
#define V_SYNC (1<<13)
  while ((V_SYNC & HOLLY.SPG_STATUS)) {
    asm volatile ("nop");
  }
  while (!(V_SYNC & HOLLY.SPG_STATUS)) {
    asm volatile ("nop");
  }
#undef V_SYNC
}

void vga()
{
  get_cable_type();

  HOLLY.SOFTRESET = 0b111;
  HOLLY.TEXT_CONTROL = 3;
  HOLLY.FB_W_CTRL = 9;

  /*
   */
  vga1();
  vga2();

  v_sync_in();

  HOLLY.VO_BORDER_COL = (63 << 5) | (31 << 0);
  HOLLY.VO_CONTROL = 0x0016;

  HOLLY.SOFTRESET = 0b000;
}

void fill_framebuffer()
{
  reg16 * vram = (reg16 *)0xa5000000;
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      struct hsv hsv = {(y * 255) / 480, 255, 255};
      struct rgb rgb = hsv_to_rgb(hsv);
      vram[y * 640 + x] = ((rgb.r >> 3) << 11) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 0);
    }
  }
  vram[0] = 0xf0ff;
  vram[10] = 0xf0ff;
}
