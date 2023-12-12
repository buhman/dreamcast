#include "type.hpp"

#include "rgb.hpp"
#include "vga.hpp"
#include "systembus.hpp"
#include "holly.hpp"

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
  unsigned char *d = dest;
  const unsigned char *s = src;
  for (; n; n--) *d++ = *s++;
  return dest;
}

extern uint32_t _binary_DSC08384_data_start __asm("_binary_DSC08384_data_start");

void start(void)
{
  /*
  G2_IF.ADEN = 0;
  G2_IF.E1EN = 0;
  G2_IF.E2EN = 0;
  G2_IF.DDEN = 0;
  G1_IF.GDEN = 0;
  MAPLE_IF.MDEN = 0;
  G2_IF.G2APRO = 0x4659404f;
  */

  holly.SOFTRESET = 0b111;
  holly.TEXT_CONTROL = 3;

  uint16_t xsize = 640;
  uint16_t ysize = 480;

  uint16_t fb_xclip_min = 0;
  uint16_t fb_xclip_max = xsize-1;
  holly.FB_X_CLIP = (fb_xclip_max << 16) | (fb_xclip_min << 0);
  uint16_t fb_yclip_min = 0;
  uint16_t fb_yclip_max = ysize-1;
  holly.FB_Y_CLIP = (fb_yclip_max << 16) | (fb_yclip_min << 0);

  uint32_t fb_xsize = (xsize * 16)/(32) - 1;
  uint32_t fb_ysize = ysize - 3;
  uint32_t fb_mod = 1;
  holly.FB_R_SIZE = 0
    | (fb_xsize << 0)
    | (fb_ysize << 10)
    | (fb_mod << 20);

  uint32_t fb_linestride = (xsize * 16) / 64;
  holly.FB_W_LINESTRIDE = fb_linestride;

  holly.FB_W_CTRL = 0
    | 0b001 << 0 // fb_packmode: RGB 565
    ;

  holly.FB_R_CTRL = 0
    | 1 << 23 // vclk_div
    | 0 << 22 // fb_strip_buf_en
    | 0 << 16 // fb_strip_size
    | 0 << 8 // fb_chroma_threshold
    | 0 << 4 // fb_concat
    | 1 << 2 // fb_depth
    | 0 << 1 // fb_line_double
    | 1 << 0 // fb_enable
    ;

  holly.FB_W_SOF1 = 0;
  holly.FB_W_SOF2 = 0;
  holly.FB_R_SOF1 = 0;
  holly.FB_R_SOF2 = 0;

  holly.VO_BORDER_COL = (31 << 11) | (31 << 0);

  uint16_t startx = 0x0a8;
  uint16_t starty = 0x028;
  holly.VO_STARTX = startx;
  holly.VO_STARTY = (starty << 16) | (starty << 0);

  holly.SPG_CONTROL = (1 << 8); // sync_direction__output ; non-default

  uint16_t hbstart = 0x0345; // default
  uint16_t hbend   = 0x007e; // default
  holly.SPG_HBLANK = (hbend << 16) | (hbstart << 0);

  uint16_t hcount = 0x0359; // default
  uint16_t vcount = 0x020c; // non-default
  holly.SPG_LOAD = (vcount << 16) | (hcount << 0);

  uint16_t vbstart = 0x0208; // non-default
  uint16_t vbend = 0x0028; // non-default
  holly.SPG_VBLANK = (vbend << 16) | (vbstart << 0);

  uint16_t hswidth = 0x003f;
  uint16_t vswidth = 0x0003;
  uint16_t bpwidth = 0x0319;
  uint16_t eqwidth = 0x000f;
  holly.SPG_WIDTH =
    (hswidth << 0)
    | (vswidth << 8)
    | (bpwidth << 12)
    | (eqwidth << 22);

  holly.SPG_HBLANK_INT = hbstart << 16;

  uint32_t hsync_pol    = 0;
  uint32_t vsync_pol    = 0;
  uint32_t blank_pol    = 0;
  uint32_t blank_video  = 0;
  uint32_t field_mode   = 0;
  uint32_t pixel_double = 0;
  uint32_t pclk_delay   = 0x16;
  holly.VO_CONTROL = 0
    | (( pclk_delay   & 0x3f) << 16 )
    | (( pixel_double & 0x01) <<  8 )
    | (( field_mode   & 0x0f) <<  4 )
    | (( blank_video  & 0x01) <<  3 )
    | (( blank_pol    & 0x01) <<  2 )
    | (( vsync_pol    & 0x01) <<  1 )
    | (( hsync_pol    & 0x01) <<  0 );


  holly.SOFTRESET = 0b000;

  const uint8_t * buf = (const uint8_t *)&_binary_DSC08384_data_start;

  reg16 * vram = (reg16 *)0xa5000000;
  v_sync_in();
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      /*
      struct hsv hsv = {(y * 255) / 480, 255, 255};
      struct rgb rgb = hsv_to_rgb(hsv);
      vram[y * 640 + x] = ((rgb.r >> 3) << 11) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 0);
      */
      //vram[y * 640 + x] = 30 << 5 | 31 << 11;
      uint32_t ix = y * 640 + x;
      uint8_t r = buf[ix * 3 + 0];
      uint8_t g = buf[ix * 3 + 1];
      uint8_t b = buf[ix * 3 + 2];
      vram[ix] = ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
    }
  }
  /*
  vram[0] = 0xffff;
  vram[10] = 0xffff;
  vram[639] = 0xffff;
  vram[307199 - (640 * 3) - 1] = 0xffff;
  vram[307199 - (640 * 2) - 10] = 31 << 11;
  vram[307199 - (640 * 1) - 1] = 0xffff;
  */
  //vram[307199 - (640 * 2) - 10] = 0xf0ff;
  //vram[307199 - 640 - 10] = 0xf0ff;
  //vram[307199 - 10] = 0xf0ff;
  //vram[307199] = 0xf0ff;

}
