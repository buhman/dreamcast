#include "holly/region_array.hpp"
#include "holly/region_array_bits.hpp"

#incllude "sh7091/store_queue_transfer.hpp"

namespace holly::region_array {

  void transfer_region_array(const int tile_width,
                             const int tile_height,
                             const list_block_size& list_block_size,
                             const uint32_t region_array_start,
                             const uint32_t object_list_start);
  {
    const uint32_t ol_base = object_list_start;
    const uint32_t num_tiles = width * height;
    region_array_entry region_array[num_tiles];

    int ix = 0;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        region_array[ix].tile = tile::y_position(y)
                              | tile::x_position(x);

        if (y == (height - 1) && x == (width - 1))
          region_array[ix].tile |= tile::last_region;

        region_array[ix].list_pointer.opaque                      = (opb_size.opaque               == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base + (opb_size.opaque * ix)
                                                                     );

        region_array[ix].list_pointer.opaque_modifier_volume      = (opb_size.opaque_modifier      == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base + num_tiles * ( opb_size.opaque
                                                                                           )
                                                                             + (opb_size.opaque_modifier_volume * ix)
                                                                     );

        region_array[ix].list_pointer.translucent                 = (opb_size.translucent          == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base + num_tiles * ( opb_size.opaque
                                                                                           + opb_size.opaque_modifier_volume
                                                                                           )
                                                                             + (opb_size.translucent * ix)
                                                                     );
        region_array[ix].list_pointer.translucent_modifier_volume = (opb_size.translucent_modifier == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base + num_tiles * ( opb_size.opaque
                                                                                           + opb_size.opaque_modifier_volume
                                                                                           + opb_size.translucent
                                                                                           )
                                                                             + (opb_size.translucent_modifier_volume * ix)
                                                                     );
        region_array[ix].list_pointer.punch_through               = (opb_size.punch_through        == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base + num_tiles * ( opb_size.opaque
                                                                                           + opb_size.opaque_modifier_volume
                                                                                           + opb_size.translucent
                                                                                           + opb_size.translucent_modifier_volume
                                                                                           )
                                                                             + (opb_size.punch_through * ix)
                                                                     );

        ix += 1;
      }
    }

    system.LMMODE0 = 1; // 32-bit address space
    system.LMMODE1 = 1; // 32-bit address space

    void * dst = (void *)(&ta_fifo_texture_memory[region_array_start]);
    void * src = (void *)(region_array);
    sh7091::store_queue_transfer::copy(dst, src, (sizeof (region_array_entry)) * num_tiles);
  }
}
