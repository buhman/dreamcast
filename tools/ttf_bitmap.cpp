#include <sstream>
#include <iostream>

#include <assert.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../twiddle.hpp"

bool is_power(int n)
{
  const int min_power = 8;
  const int max_power = 1024;
  int power = min_power;

  while (power <= max_power) {
    if (n == power)
      return true;
    power *= 2;
  }
  printf("%d\n", n);
  return false;
}

int32_t
load_bitmap_char(FT_Face face,
		 FT_ULong char_code,
		 uint8_t * buf)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    std::cerr << "FT_Load_Glyph " << FT_Error_String(error) << '\n';
    return -1;
  }

  assert(face->glyph->format == FT_GLYPH_FORMAT_BITMAP);
  //printf("num_grays %d\n", face->glyph->bitmap.num_grays);
  //printf("pitch %d\n", face->glyph->bitmap.pitch);
  //printf("width %d\n", face->glyph->bitmap.width);
  //printf("rows %d\n", face->glyph->bitmap.rows);
  assert(face->glyph->bitmap.num_grays == 2);
  assert(is_power(face->glyph->bitmap.rows));
  assert(is_power(face->glyph->bitmap.width));
  assert((int)(face->glyph->bitmap.pitch * 8) == (int)(face->glyph->bitmap.width));

  for (int y = 0; y < (int)face->glyph->bitmap.rows; y++) {
    uint8_t * row = &face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch];
    for (int x = 0; x < (int)face->glyph->bitmap.width; x += 1) {
      const int bit = (row[x / 8] >> (7 - (x % 8))) & 1;
      //std::cerr << (bit ? "█" : " ");
      buf[y * face->glyph->bitmap.width + x] = bit;
    }
    //std::cerr << "|\n";
  }

  // 'pitch' is bytes; 'width' is pixels
  return face->glyph->bitmap.rows * face->glyph->bitmap.width;
}

template <typename T>
constexpr inline T
parse_num(decltype(std::hex)& format, const char * s)
{
  T n;
  std::stringstream ss;
  ss << format << s;
  ss >> n;
  return n;
}

int main(int argc, char *argv[])
{
  FT_Library library;
  FT_Face face;
  FT_Error error;

  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " [start-hex] [end-hex] [font-file-path] [output-file-path]\n\n";
    std::cerr << "   ex: " << argv[0] << " 3000 30ff ipagp.ttf font.bin\n";
    return -1;
  }

  auto start = parse_num<uint32_t>(std::hex, argv[1]);
  auto end = parse_num<uint32_t>(std::hex, argv[2]);
  auto font_file_path = argv[3];
  auto output_file_path = argv[4];

  error = FT_Init_FreeType(&library);
  if (error) {
    std::cerr << "FT_Init_FreeType\n";
    return -1;
  }

  error = FT_New_Face(library, font_file_path, 0, &face);
  if (error) {
    std::cerr << "FT_New_Face\n";
    return -1;
  }

  error = FT_Select_Size(face, 0);
  if (error) {
    std::cerr << "FT_Select_Size: " << FT_Error_String(error) << ' ' << error << '\n';
    return -1;
  }

  assert(end > start);

  int width = face->size->metrics.max_advance >> 6;
  int height = face->size->metrics.height >> 6;
  uint32_t pixels_per_glyph = width * height;
  assert(pixels_per_glyph % 8 == 0);
  uint32_t bytes_per_glyph = pixels_per_glyph;
  uint32_t num_glyphs = (end - start) + 1;
  uint32_t buf_size = bytes_per_glyph * num_glyphs;
  uint8_t * buf = (uint8_t *)malloc(buf_size);

  uint8_t * twiddle_buf = (uint8_t *)malloc(buf_size / 2);

  uint32_t bitmap_offset = 0;
  for (uint32_t char_code = start; char_code <= end; char_code++) {
    int32_t bitmap_size = load_bitmap_char(face,
					   char_code,
					   &buf[bitmap_offset]);
    if (bitmap_size < 0) {
      std::cerr << "load_bitmap_char error\n";
      return -1;
    }

    printf("twiddle %d %d %d %d\n", bitmap_offset, bitmap_offset / 2, width, height);
    twiddle::texture_4bpp(&twiddle_buf[bitmap_offset / 2],
                          &buf[bitmap_offset],
                          width,
                          height);

    bitmap_offset += bitmap_size;
    assert(bitmap_offset <= buf_size);
  }
  printf("bitmap_offset: %d\n", bitmap_offset);

  FILE * out = fopen(output_file_path, "w");
  if (out == NULL) {
    perror("fopen(w)");
    return -1;
  }
  fwrite(reinterpret_cast<void*>(twiddle_buf), bitmap_offset / 2, 1, out);
  fclose(out);
}
