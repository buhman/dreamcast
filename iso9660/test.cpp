#include <fstream>
#include <string>
#include <iostream>

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

    if (dr->file_flags == 0) {
      std::cout << "location_of_extent: " << dr->location_of_extent.get() << '\n';
      std::cout << "data_length: " << std::dec << dr->data_length.get() << '\n';

      if (dr->file_identifier[0] != '1') {
	const uint32_t extent = dr->location_of_extent.get() * 2048;
	auto file = reinterpret_cast<const uint8_t *>(&str[extent]);
	std::cout << "---begin file content---\n";
	write_field(file, dr->data_length.get());
	std::cout << "---end file content---\n";
      }
    }

    offset += dr->length_of_directory_record;
  }

  return 0;
}
