#pragma once

#include <cstdint>
#include <cstddef>

#include "uint_le_be.hpp"

namespace iso9660 {
  struct primary_volume_descriptor {
    const uint8_t volume_descriptor_type;
    const uint8_t standard_identifier[5];
    const uint8_t volume_descriptor_version;
    const uint8_t _res1;
    const uint8_t system_identifier[32];
    const uint8_t volume_identifier[32];
    const uint8_t _res2[8];
    const uint32_le_be volume_space_size;
    const uint8_t _res3[32];
    const uint16_le_be volume_set_size;
    const uint16_le_be volume_sequence_number;
    const uint16_le_be logical_block_size;
    const uint32_le_be path_table_size;
    const uint16_le_be location_of_occurrence_of_type_l_path_table;
    const uint16_le_be location_of_optional_occurence_of_type_l_path_table;
    const uint16_le_be location_of_occurence_of_type_m_path_table;
    const uint16_le_be location_of_optional_occurence_of_type_m_path_table;
    const uint8_t directory_record_for_root_directory[34];
    const uint8_t volume_set_identifier[128];
    const uint8_t publisher_identifier[128];
    const uint8_t data_preparer_identifier[128];
    const uint8_t application_identifier[128];
    const uint8_t copyright_file_identifier[37];
    const uint8_t abstract_file_identifier[37];
    const uint8_t bibliographic_file_identifier[37];
    const uint8_t volume_creation_date_and_time[17];
    const uint8_t volume_modification_date_and_time[17];
    const uint8_t volume_expiration_date_and_time[17];
    const uint8_t volume_effective_date_and_time[17];
    const uint8_t file_structure_version;
    const uint8_t _res4;
    const uint8_t application_use[512];
    const uint8_t _res5[653];
  };
  static_assert((offsetof (struct primary_volume_descriptor, volume_descriptor_type)) == 0);
  static_assert((offsetof (struct primary_volume_descriptor, standard_identifier)) == 1);
  static_assert((offsetof (struct primary_volume_descriptor, volume_descriptor_version)) == 6);
  static_assert((offsetof (struct primary_volume_descriptor, _res1)) == 7);
  static_assert((offsetof (struct primary_volume_descriptor, system_identifier)) == 8);
  static_assert((offsetof (struct primary_volume_descriptor, volume_identifier)) == 40);
  static_assert((offsetof (struct primary_volume_descriptor, _res2)) == 72);
  static_assert((offsetof (struct primary_volume_descriptor, volume_space_size)) == 80);
  static_assert((offsetof (struct primary_volume_descriptor, _res3)) == 88);
  static_assert((offsetof (struct primary_volume_descriptor, volume_set_size)) == 120);
  static_assert((offsetof (struct primary_volume_descriptor, volume_sequence_number)) == 124);
  static_assert((offsetof (struct primary_volume_descriptor, logical_block_size)) == 128);
  static_assert((offsetof (struct primary_volume_descriptor, path_table_size)) == 132);
  static_assert((offsetof (struct primary_volume_descriptor, location_of_occurrence_of_type_l_path_table)) == 140);
  static_assert((offsetof (struct primary_volume_descriptor, location_of_optional_occurence_of_type_l_path_table)) == 144);
  static_assert((offsetof (struct primary_volume_descriptor, location_of_occurence_of_type_m_path_table)) == 148);
  static_assert((offsetof (struct primary_volume_descriptor, location_of_optional_occurence_of_type_m_path_table)) == 152);
  static_assert((offsetof (struct primary_volume_descriptor, directory_record_for_root_directory)) == 156);
  static_assert((offsetof (struct primary_volume_descriptor, volume_set_identifier)) == 190);
  static_assert((offsetof (struct primary_volume_descriptor, publisher_identifier)) == 318);
  static_assert((offsetof (struct primary_volume_descriptor, data_preparer_identifier)) == 446);
  static_assert((offsetof (struct primary_volume_descriptor, application_identifier)) == 574);
  static_assert((offsetof (struct primary_volume_descriptor, copyright_file_identifier)) == 702);
  static_assert((offsetof (struct primary_volume_descriptor, abstract_file_identifier)) == 739);
  static_assert((offsetof (struct primary_volume_descriptor, bibliographic_file_identifier)) == 776);
  static_assert((offsetof (struct primary_volume_descriptor, volume_creation_date_and_time)) == 813);
  static_assert((offsetof (struct primary_volume_descriptor, volume_modification_date_and_time)) == 830);
  static_assert((offsetof (struct primary_volume_descriptor, volume_expiration_date_and_time)) == 847);
  static_assert((offsetof (struct primary_volume_descriptor, volume_effective_date_and_time)) == 864);
  static_assert((offsetof (struct primary_volume_descriptor, file_structure_version)) == 881);
  static_assert((offsetof (struct primary_volume_descriptor, _res4)) == 882);
  static_assert((offsetof (struct primary_volume_descriptor, application_use)) == 883);
  static_assert((offsetof (struct primary_volume_descriptor, _res5)) == 1395);
}
