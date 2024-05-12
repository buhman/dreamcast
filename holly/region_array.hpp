#pragma once

#include <cstdint>

struct opb_size {
  uint32_t opaque;
  uint32_t opaque_modifier;
  uint32_t translucent;
  uint32_t translucent_modifier;
  uint32_t punch_through;

  uint32_t total() const
  {
    return opaque
         + opaque_modifier
	 + translucent
	 + translucent_modifier
	 + punch_through;
  }
};

void region_array(const uint32_t width,   // in tile units (1 tile unit = 32 pixels)
		  const uint32_t height); // in tile units (1 tile unit = 32 pixels)

void region_array2(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                   const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                   const struct opb_size& opb_size);

void region_array_multipass(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                            const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                            const struct opb_size * opb_size,
                            const uint32_t num_render_passes);
