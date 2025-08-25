#pragma once

#include <cstdint>

namespace holly::core::region_array {
  struct region_array_entry {
    uint32_t tile;
    struct {
      uint32_t opaque;
      uint32_t opaque_modifier_volume;
      uint32_t translucent;
      uint32_t translucent_modifier_volume;
      uint32_t punch_through;
    } list_pointer;
  };

  struct list_block_size {
    uint32_t opaque;
    uint32_t opaque_modifier_volume;
    uint32_t translucent;
    uint32_t translucent_modifier_volume;
    uint32_t punch_through;

    uint32_t total() const
    {
      return opaque
        + opaque_modifier_volume
        + translucent
        + translucent_modifier_volume
        + punch_through;
    }
  };

  void transfer(const int tile_width,
                const int tile_height,
                const list_block_size& list_block_size,
                const uint32_t region_array_start,
                const uint32_t object_list_start);
}
