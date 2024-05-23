#pragma once

namespace ft1 {
  namespace get_media_info_data_transfer {
    struct data_format {
      uint16_t total_size;
      uint16_t partition_number;
      uint16_t system_area_block_number;
      uint16_t fat_area_block_number;
      uint16_t number_of_fat_area_blocks;
      uint16_t file_information_block_number;
      uint16_t number_of_file_information_blocks;
      uint16_t volume_icon;
      uint16_t save_area_block_number;
      uint16_t number_of_save_area_blocks;
      uint32_t reserved_for_execution_file;
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 24);
  }

  namespace block_read_data_transfer {
    struct data_format {
      uint8_t pt;
      uint8_t phase;
      uint16_t block_number;
      uint8_t block_data[0];
    };
    static_assert((sizeof (struct data_format)) % 4 == 0);
    static_assert((sizeof (struct data_format)) == 4);
  }

}

