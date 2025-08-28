#include "holly/core/region_array.hpp"
#include "holly/core/region_array_bits.hpp"

#include "sh7091/store_queue_transfer.hpp"

#include "systembus/systembus.hpp"

namespace holly::core::region_array {
  void transfer(const int tile_width,
                const int tile_height,
                const list_block_size& list_block_size,
                const uint32_t region_array_start,
                const uint32_t object_list_start)
  {
    const uint32_t ol_base = object_list_start;
    const uint32_t num_tiles = tile_width * tile_height;
    //region_array_entry region_array[num_tiles];
    volatile region_array_entry * region_array = (volatile region_array_entry * )&texture_memory32[region_array_start];

    int ix = 0;

    for (int y = 0; y < tile_height; y++) {
      for (int x = 0; x < tile_width; x++) {
        int rix = x * tile_height + y;
        //int rix = y * tile_width + x;

        region_array[rix].tile = tile::y_position(y)
                              | tile::x_position(x);

        if (y == (tile_height - 1) && x == (tile_width - 1))
          region_array[rix].tile |= tile::last_region;

        region_array[rix].list_pointer.opaque                      = (list_block_size.opaque                      == 0) ? list_pointer::empty :
          (ol_base + (list_block_size.opaque * ix)
           );

        region_array[rix].list_pointer.opaque_modifier_volume      = (list_block_size.opaque_modifier_volume      == 0) ? list_pointer::empty :
          (ol_base + num_tiles * ( list_block_size.opaque
                                 )
                   + (list_block_size.opaque_modifier_volume * ix)
           );

        region_array[rix].list_pointer.translucent                 = (list_block_size.translucent                 == 0) ? list_pointer::empty :
          (ol_base + num_tiles * ( list_block_size.opaque
                                 + list_block_size.opaque_modifier_volume
                                 )
                   + (list_block_size.translucent * ix)
           );
        region_array[rix].list_pointer.translucent_modifier_volume = (list_block_size.translucent_modifier_volume == 0) ? list_pointer::empty :
          (ol_base + num_tiles * ( list_block_size.opaque
                                 + list_block_size.opaque_modifier_volume
                                 + list_block_size.translucent
                                 )
                   + (list_block_size.translucent_modifier_volume * ix)
           );
        region_array[rix].list_pointer.punch_through               = (list_block_size.punch_through               == 0) ? list_pointer::empty :
          (ol_base + num_tiles * ( list_block_size.opaque
                                 + list_block_size.opaque_modifier_volume
                                 + list_block_size.translucent
                                 + list_block_size.translucent_modifier_volume
                                 )
                   + (list_block_size.punch_through * ix)
           );

        ix += 1;
      }
    }

    /*
    using systembus::systembus;

    systembus.LMMODE0 = 1; // 32-bit address space
    systembus.LMMODE1 = 1; // 32-bit address space

    void * dst = (void *)(&ta_fifo_texture_memory[region_array_start]);
    void * src = (void *)(region_array);
    sh7091::store_queue_transfer::copy(dst, src, (sizeof (region_array_entry)) * num_tiles);
    */
  }
}
