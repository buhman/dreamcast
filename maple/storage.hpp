#include <cstdint>
#include <cstddef>

namespace storage {

struct system_area {
  uint8_t format_information[16];
  uint8_t volume_label[32];
  uint8_t date_and_time_created[8];
  uint8_t _reserved0[8];
  uint16_t total_size;
  uint16_t partition_number;
  uint16_t system_area_block_number;
  uint16_t fat_area_block_number;
  uint16_t number_of_fat_area_blocks;
  uint16_t file_information_block_number;
  uint16_t number_of_file_information_blocks;
  uint8_t volume_icon;
  uint8_t reserved;
  uint16_t save_area_block_number;
  uint16_t number_of_save_area_blocks;
  uint8_t reserved_for_execution_file[4];
  uint8_t _reserved1[8];
  uint8_t _reserved2[416];
};

static_assert((sizeof (struct system_area)) == 0x200);
static_assert((offsetof (struct system_area, format_information)) == 0x000);
static_assert((offsetof (struct system_area, volume_label)) == 0x010);
static_assert((offsetof (struct system_area, date_and_time_created)) == 0x030);
static_assert((offsetof (struct system_area, total_size)) == 0x040);
static_assert((offsetof (struct system_area, save_area_block_number)) == 0x050);
static_assert((offsetof (struct system_area, _reserved2)) == 0x060);

struct fat_area {
  uint16_t fat_number[0];

  struct data {
    static constexpr uint16_t data_end = 0xfffa;
    static constexpr uint16_t unused = 0xfffc;
    static constexpr uint16_t block_damaged = 0xffff;
  };
};

static_assert((sizeof (struct fat_area)) == 0);

struct file_information {
  uint8_t status;
  uint8_t copy;
  uint16_t start_fat;
  uint8_t file_name[12];
  uint8_t date[8];
  uint16_t block_size;
  uint16_t header;
  uint32_t _reserved;

  struct status {
    static constexpr uint16_t no_data_file = 0x00;
    static constexpr uint16_t data_file = 0x33;
    static constexpr uint16_t execution_file = 0xcc;
  };
};

static_assert((sizeof (struct file_information)) == 32);

}
