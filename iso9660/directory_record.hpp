#pragma once

#include <cstdint>
#include <cstddef>

#include "uint_le_be.hpp"

namespace iso9660 {
  struct directory_record {
    const uint8_t length_of_directory_record;
    const uint8_t extended_attribute_record_length;
    const uint32_le_be location_of_extent;
    const uint32_le_be data_length;
    const uint8_t recording_date_and_time[7];
    const uint8_t file_flags;
    const uint8_t file_unit_size;
    const uint8_t interleave_gap_size;
    const uint16_le_be volume_sequence_number;
    const uint8_t length_of_file_identifier;
    const uint8_t file_identifier[];
  };
  static_assert((offsetof (struct directory_record, length_of_directory_record)) == 0);
  static_assert((offsetof (struct directory_record, extended_attribute_record_length)) == 1);
  static_assert((offsetof (struct directory_record, location_of_extent)) == 2);
  static_assert((offsetof (struct directory_record, data_length)) == 10);
  static_assert((offsetof (struct directory_record, recording_date_and_time)) == 18);
  static_assert((offsetof (struct directory_record, file_flags)) == 25);
  static_assert((offsetof (struct directory_record, file_unit_size)) == 26);
  static_assert((offsetof (struct directory_record, interleave_gap_size)) == 27);
  static_assert((offsetof (struct directory_record, volume_sequence_number)) == 28);
  static_assert((offsetof (struct directory_record, length_of_file_identifier)) == 32);
  static_assert((offsetof (struct directory_record, file_identifier)) == 33);
}
