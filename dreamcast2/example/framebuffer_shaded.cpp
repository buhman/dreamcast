#include "memorymap.hpp"
#include "holly/holly.hpp"
#include "holly/holly_bits.hpp"

void main()
{
  volatile uint32_t * framebuffer = (volatile uint32_t * )texture_memory32;

  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      int red = (y % 255);
      int blue = (x % 255);

      framebuffer[y * 640 + x] = (red << 16) | (blue << 0);
    }
  }

  using holly::holly;
  using namespace holly;

  holly.FB_R_CTRL
    = fb_r_ctrl::vclk_div::pclk_vclk_1
    | fb_r_ctrl::fb_depth::xrgb0888
    | fb_r_ctrl::fb_enable;

  int fb_y_size = 480 - 1;
  int bytes_per_pixel = 4;
  int fb_x_size = ((640 * bytes_per_pixel) / 4) - 1;
  holly.FB_R_SIZE
    = fb_r_size::fb_modulus(1)
    | fb_r_size::fb_y_size(fb_y_size)
    | fb_r_size::fb_x_size(fb_x_size);

  // the framebuffer is at the start of texture memory (texture memory address 0)
  holly.FB_R_SOF1 = 0;
}
