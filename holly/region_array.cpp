#include <cstdint>

#include "region_array.hpp"

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

// opaque list pointer offset: OPB size * tile index * 4

void region_array(volatile uint32_t * buf,
                  const uint32_t ol_base,
                  const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                  const uint32_t height) // in tile units (1 tile unit = 32 pixels)
{
  volatile region_array_entry * region_array = reinterpret_cast<volatile region_array_entry *>(buf);
  uint32_t ix = 0;

  // create a "dummy region array [item]" for CORE & TA-related bug #21:
  //  "Misshapen tiles or missing tiles occur"
  /*
  region_array[ix].tile = REGION_ARRAY__FLUSH_ACCUMULATE;
  region_array[ix].opaque_list_pointer                      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_list_pointer                 = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;
  ix += 1;
  */

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      region_array[ix].tile = REGION_ARRAY__TILE_Y_POSITION(y)
                            | REGION_ARRAY__TILE_X_POSITION(x);

      if (y == (height - 1) && x == (width - 1))
        region_array[ix].tile |= REGION_ARRAY__LAST_REGION;

      uint32_t tile_index = y * width + x;
      constexpr uint32_t opaque_list_opb_size = 16 * 4; // in bytes; this must match O_OPB in TA_ALLOC_CTRL
      region_array[ix].opaque_list_pointer                      = ol_base + (opaque_list_opb_size * tile_index);
      region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
      region_array[ix].translucent_list_pointer                 = REGION_ARRAY__LIST_POINTER__EMPTY;
      region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
      region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;

      ix += 1;
    }
  }
}

void region_array2(volatile uint32_t * buf,
                   const uint32_t ol_base,
                   const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                   const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                   const struct opb_size& opb_size)
{
  volatile region_array_entry * region_array = reinterpret_cast<volatile region_array_entry *>(buf);
  const uint32_t num_tiles = width * height;
  uint32_t ix = 0;

  /*
  region_array[ix].tile = REGION_ARRAY__FLUSH_ACCUMULATE;
  region_array[ix].opaque_list_pointer                      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_list_pointer                 = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;
  ix += 1;
  */

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      region_array[ix].tile = REGION_ARRAY__TILE_Y_POSITION(y)
                            | REGION_ARRAY__TILE_X_POSITION(x);

      if (y == (height - 1) && x == (width - 1))
        region_array[ix].tile |= REGION_ARRAY__LAST_REGION;

      uint32_t tile_index = y * width + x;
      region_array[ix].opaque_list_pointer                      = (opb_size.opaque               == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                  (ol_base + (opb_size.opaque * tile_index)
                                                                   );

      region_array[ix].opaque_modifier_volume_list_pointer      = (opb_size.opaque_modifier      == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                  (ol_base + num_tiles * ( opb_size.opaque
											 )
                                                                           + (opb_size.opaque_modifier * tile_index)
                                                                   );

      region_array[ix].translucent_list_pointer                 = (opb_size.translucent          == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                  (ol_base + num_tiles * ( opb_size.opaque
											 + opb_size.opaque_modifier
											 )
                                                                           + (opb_size.translucent * tile_index)
                                                                   );
      region_array[ix].translucent_modifier_volume_list_pointer = (opb_size.translucent_modifier == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                  (ol_base + num_tiles * ( opb_size.opaque
											 + opb_size.opaque_modifier
											 + opb_size.translucent
											 )
                                                                           + (opb_size.translucent_modifier * tile_index)
                                                                   );
      region_array[ix].punch_through_list_pointer               = (opb_size.punch_through        == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                  (ol_base + num_tiles * ( opb_size.opaque
											 + opb_size.opaque_modifier
											 + opb_size.translucent
											 + opb_size.translucent_modifier
											 )
                                                                           + (opb_size.punch_through * tile_index)
                                                                   );

      ix += 1;
    }
  }
}

void region_array_multipass(volatile uint32_t * buf,
                            const uint32_t ol_base,
                            const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                            const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                            const uint32_t num_render_passes)
{
  volatile region_array_entry * region_array = reinterpret_cast<volatile region_array_entry *>(buf);
  uint32_t ix = 0;

  /*
  // create a "dummy region array [item]" for CORE & TA-related bug #21:
  //  "Misshapen tiles or missing tiles occur"
  region_array[ix].tile = REGION_ARRAY__FLUSH_ACCUMULATE;
  region_array[ix].opaque_list_pointer                      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_list_pointer                 = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
  region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;
  ix += 1;
  */

  constexpr uint32_t list_opb_size = 16 * 4; // for a single OPB in bytes; this must match O_OPB in TA_ALLOC_CTRL
  const uint32_t opb_render_pass_size = width * height * list_opb_size; // the sum of the size of all OPB for a single pass

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {

      region_array[ix].tile = REGION_ARRAY__TILE_Y_POSITION(y)
                            | REGION_ARRAY__TILE_X_POSITION(x);

      /* 0x10 = FLUSH_ACCUMULATE
         0x50 = FLUSH_ACCUMULATE | Z_CLEAR
         0x40 = Z_CLEAR
      */

      for (uint32_t render_pass = 0; render_pass < num_render_passes; render_pass++) {
        if (render_pass != (num_render_passes - 1))
          region_array[ix].tile |= REGION_ARRAY__FLUSH_ACCUMULATE;

        if (render_pass > 0)
          region_array[ix].tile |= REGION_ARRAY__Z_CLEAR;

        uint32_t tile_index = y * width + x;
        uint32_t pass_ol_base = ol_base + (opb_render_pass_size * render_pass);
        if (render_pass == 0) {
          region_array[ix].opaque_list_pointer                      = pass_ol_base + (list_opb_size * tile_index);
          region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].translucent_list_pointer                 = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;
        } else {
          // (list_opb_size * width * height) +
          region_array[ix].opaque_list_pointer                      = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].opaque_modifier_volume_list_pointer      = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].translucent_list_pointer                 = pass_ol_base + (list_opb_size * tile_index);
          region_array[ix].translucent_modifier_volume_list_pointer = REGION_ARRAY__LIST_POINTER__EMPTY;
          region_array[ix].punch_through_list_pointer               = REGION_ARRAY__LIST_POINTER__EMPTY;
        }

        if (y == (height - 1) && x == (width - 1))
          region_array[ix].tile |= REGION_ARRAY__LAST_REGION;

        ix += 1;
      }
    }
  }
}
