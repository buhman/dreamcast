#include <cstdint>

#include "region_array.hpp"
#include "texture_memory_alloc.hpp"
#include "memorymap.hpp"

// opaque list pointer offset: OPB size * tile index * 4

void region_array(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                  const uint32_t height) // in tile units (1 tile unit = 32 pixels)
{
  auto region_array = reinterpret_cast<volatile region_array_entry *>
    (&texture_memory32[texture_memory_alloc::region_array.start / 4]);

  const uint32_t ol_base = texture_memory_alloc::object_list.start;

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

void region_array2(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                   const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                   const struct opb_size& opb_size)
{
  auto region_array = reinterpret_cast<volatile region_array_entry *>
    (&texture_memory32[texture_memory_alloc::region_array.start / 4]);

  const uint32_t ol_base = texture_memory_alloc::object_list.start;

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

void region_array_multipass(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                            const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                            const struct opb_size * opb_size,
                            const uint32_t num_render_passes,
			    const uint32_t region_array_start,
			    const uint32_t object_list_start)
{
  auto region_array = reinterpret_cast<volatile region_array_entry *>
    (&texture_memory32[region_array_start / 4]);

  const uint32_t num_tiles = width * height;
  uint32_t ol_base[num_render_passes];

  ol_base[0] = object_list_start;
  for (uint32_t pass = 1; pass < num_render_passes; pass++) {
    ol_base[pass] = ol_base[pass - 1] + num_tiles * opb_size[pass - 1].total();
  }

  uint32_t ix = 0;

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      for (uint32_t pass = 0; pass < num_render_passes; pass++) {
        region_array[ix].tile = REGION_ARRAY__TILE_Y_POSITION(y)
                              | REGION_ARRAY__TILE_X_POSITION(x);

        region_array[ix].tile |= REGION_ARRAY__PRE_SORT;

        if (pass == (num_render_passes - 1) && y == (height - 1) && x == (width - 1))
          region_array[ix].tile |= REGION_ARRAY__LAST_REGION;

        if (pass != (num_render_passes - 1))
          region_array[ix].tile |= REGION_ARRAY__FLUSH_ACCUMULATE;

        if (pass > 0)
          region_array[ix].tile |= REGION_ARRAY__Z_CLEAR;

        uint32_t tile_index = y * width + x;
        region_array[ix].opaque_list_pointer                      = (opb_size[pass].opaque               == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + (opb_size[pass].opaque * tile_index)
                                                                     );

        region_array[ix].opaque_modifier_volume_list_pointer      = (opb_size[pass].opaque_modifier      == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 )
                                                                                   + (opb_size[pass].opaque_modifier * tile_index)
                                                                     );

        region_array[ix].translucent_list_pointer                 = (opb_size[pass].translucent          == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 )
                                                                                   + (opb_size[pass].translucent * tile_index)
                                                                     );
        region_array[ix].translucent_modifier_volume_list_pointer = (opb_size[pass].translucent_modifier == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 + opb_size[pass].translucent
                                                                                                 )
                                                                                   + (opb_size[pass].translucent_modifier * tile_index)
                                                                     );
        region_array[ix].punch_through_list_pointer               = (opb_size[pass].punch_through        == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 + opb_size[pass].translucent
                                                                                                 + opb_size[pass].translucent_modifier
                                                                                                 )
                                                                                   + (opb_size[pass].punch_through * tile_index)
                                                                     );
        ix += 1;
      }
    }
  }
}
