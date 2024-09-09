#pragma once

#include <cstdint>

#define REGION_ARRAY__LAST_REGION (1 << 31)
#define REGION_ARRAY__Z_CLEAR (1 << 30)
#define REGION_ARRAY__PRE_SORT (1 << 29)
#define REGION_ARRAY__FLUSH_ACCUMULATE (1 << 28)
#define REGION_ARRAY__TILE_Y_POSITION(n) (((n) & 0x3f) << 8)
#define REGION_ARRAY__TILE_X_POSITION(n) (((n) & 0x3f) << 2)

#define REGION_ARRAY__LIST_POINTER__EMPTY (1 << 31)
#define REGION_ARRAY__LIST_POINTER(n) ((n) & 0xfffffc)

// this is for a "type 2" region array.
// region header type is specified in FPU_PARAM_CFG
struct region_array_entry {
  uint32_t tile; /* 3.7.7 page 216 */
  uint32_t opaque_list_pointer;
  uint32_t opaque_modifier_volume_list_pointer;
  uint32_t translucent_list_pointer;
  uint32_t translucent_modifier_volume_list_pointer;
  uint32_t punch_through_list_pointer;
};

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
                            const uint32_t num_render_passes,
			    const uint32_t region_array_start,
			    const uint32_t object_list_start);
