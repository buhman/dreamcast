#include "holly/video_output.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "memorymap.hpp"

void main()
{
  video_output::set_mode_vga();

  uint32_t frame_address = texture_memory_alloc::framebuffer[0].start;
  volatile uint16_t * framebuffer = reinterpret_cast<volatile uint16_t*>(&texture_memory32[frame_address/4]);

  for (int i = 0; i < 640 * 480; i++) {
    if (i % 2)
      framebuffer[i] = 31 << 11;
    else
      framebuffer[i] = 0;
  }

  holly.FB_R_SOF1 = frame_address;
}
