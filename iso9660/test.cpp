#include <fstream>
#include <string>
#include <iostream>
#include <bit>
#include <cassert>

#include "primary_volume_descriptor.hpp"
#include "directory_record.hpp"

void write_field(const uint8_t * s, const int len)
{
  for (int i = 0; i < len; i++) {
    std::cout << s[i];
  }
  std::cout << '\n';
}

int main()
{
  using namespace iso9660;

  std::ifstream ifs("test.iso");
  std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()    ) );

  const char * str = content.c_str();
  auto pvd = reinterpret_cast<const primary_volume_descriptor *>(&str[2048 * 16]);

  write_field(pvd->standard_identifier, 5);

  auto root_dr = reinterpret_cast<const directory_record *>(&pvd->directory_record_for_root_directory);

  std::cout << "root directory record:" << '\n';
  std::cout << root_dr->location_of_extent.get() << '\n';
  std::cout << root_dr->data_length.get() << '\n';

  uint32_t offset = root_dr->location_of_extent.get() * 2048;
  while (true) {
    std::cout << "\n\n";
    std::cout << "directory entry offset: " << std::hex << offset << '\n';
    auto dr = reinterpret_cast<const directory_record *>(&str[offset]);
    if (dr->length_of_directory_record == 0)
      break;

    std::cout << "length_of_directory_record: ";
    std::cout << std::dec << (int)dr->length_of_directory_record << '\n';
    std::cout << "length_of_file_identifier: ";
    std::cout << (int)dr->length_of_file_identifier << '\n';
    std::cout << "file_identifier: ";
    write_field(dr->file_identifier, dr->length_of_file_identifier);
    std::cout << std::hex << "file_flags: " << (int)dr->file_flags << '\n';

    uint32_t system_use_offset = 33 + dr->length_of_file_identifier + ((dr->length_of_file_identifier & 1) == 0);
    const char * system_use = &str[offset + system_use_offset];
    std::cout << "system use:\n";

    for (int i = 0; i < dr->length_of_directory_record - system_use_offset;) {
      std::cout << "  entry: ";
      std::cout << system_use[i + 0] << system_use[i + 1] << '\n';

      int length = (uint8_t)system_use[i + 2];
      std::cout << "  length: ";
      std::cout << length << '\n';

      if (system_use[i + 0] == 'N' && system_use[i + 1] == 'M') {
        int version = (uint8_t)system_use[i + 3];
        assert(version == 1);
        int flags = (uint8_t)system_use[i + 4];
        if (flags == 0) {
          std::cout << "name: ";
          for (int j = 5; j < length; j++) {
            std::cout << system_use[i + j];
          }
          std::cout << '\n';
        }
      }

      if (length == 0) length = 1;
      i += length;
    }
    std::cout << '\n';

    if (dr->file_flags == 0) {
      std::cout << "location_of_extent: " << dr->location_of_extent.get() << '\n';
      std::cout << "data_length: " << std::dec << dr->data_length.get() << '\n';

      /*
      if (dr->file_identifier[0] != '1') {
	const uint32_t extent = dr->location_of_extent.get() * 2048;
	auto file = reinterpret_cast<const uint8_t *>(&str[extent]);
	std::cout << "---begin file content---\n";
	write_field(file, dr->data_length.get());
	std::cout << "---end file content---\n";
      }
      */
    }

    offset += dr->length_of_directory_record;
  }

  return 0;
}
