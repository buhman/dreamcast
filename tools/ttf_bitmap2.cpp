#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../twiddle.hpp"

int
load_bitmap_char(FT_Face face,
		 FT_ULong char_code,
                 int buf_stride,
		 uint8_t * buf)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    fprintf(stderr, "FT_Load_Glyph %s\n", FT_Error_String(error));
    return -1;
  }

  printf("horiBearingX: %d\n", face->glyph->metrics.horiBearingX >> 6);
  printf("horiBearingY: %d\n", face->glyph->metrics.horiBearingY >> 6);
  printf("horiAdvance: %d\n", face->glyph->metrics.horiAdvance >> 6);

  assert(face->glyph->format == FT_GLYPH_FORMAT_BITMAP);
  assert(face->glyph->bitmap.num_grays == 2);

  for (int y = 0; y < (int)face->glyph->bitmap.rows; y++) {
    uint8_t * row = &face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch];
    for (int x = 0; x < (int)face->glyph->bitmap.width; x += 1) {
      const int bit = (row[x / 8] >> (7 - (x % 8))) & 1;
      //std::cerr << (bit ? "█" : " ");
      buf[y * buf_stride + x] = bit;
    }
    //std::cerr << "|\n";
  }

  return 0;
}

int load_font(FT_Library * library, FT_Face * face, const char * font_file_path)
{
  FT_Error error;

  error = FT_Init_FreeType(library);
  if (error) {
    fprintf(stderr, "FT_Init_FreeType\n");
    return -1;
  }

  error = FT_New_Face(*library, font_file_path, 0, face);
  if (error) {
    fprintf(stderr, "FT_New_Face\n");
    return -1;
  }

  error = FT_Select_Size(*face, 0);
  if (error) {
    fprintf(stderr, "FT_Select_Size: %d: %s\n", error, FT_Error_String(error));
    return -1;
  }

  return 0;
}

void usage(const char * argv_0)
{
  printf("%s [start-hex] [end-hex] [texture-width] [texture-height] [font-file-path] [output-file-path]\n", argv_0);
}

int main(int argc, const char * argv[])
{
  if (argc != 7) {
    usage(argv[0]);
    return -1;
  }

  char * endptr;

  int start_hex = strtol(argv[1], &endptr, 16);
  assert(*endptr == 0);
  int end_hex = strtol(argv[2], &endptr, 16);
  assert(*endptr == 0);

  int texture_width = strtol(argv[3], &endptr, 10);
  assert(*endptr == 0);
  int texture_height = strtol(argv[4], &endptr, 10);
  assert(*endptr == 0);

  const char * font_file_path = argv[5];
  const char * output_file_path = argv[6];

  printf("start_hex %x\n", start_hex);
  printf("end_hex %x\n", start_hex);
  printf("texture_width %d\n", texture_width);
  printf("texture_height %d\n", texture_height);
  printf("font_file_path %s\n", font_file_path);
  printf("output_file_path %s\n", output_file_path);

  FT_Library library;
  FT_Face face;
  int res;
  res = load_font(&library, &face, font_file_path);
  if (res < 0)
    return -1;

  int width = face->size->metrics.max_advance >> 6;
  int height = face->size->metrics.height >> 6;

  printf("width %d\n", width);
  printf("height %d\n", height);

  int texture_buf_size = texture_width * texture_height;
  uint8_t * texture = (uint8_t *)malloc(texture_buf_size);

  int x = 0;
  int y = 0;

  for (int char_code = start_hex; char_code <= end_hex; char_code++) {
    res = load_bitmap_char(face,
                           char_code,
                           texture_width,
                           &texture[y * texture_width + x]);
    x += width;
    if (x + width > texture_width) {
      y += height;
      x = 0;
      assert(y + height <= texture_height);
    }
  }

  uint8_t * twiddle = (uint8_t *)malloc(texture_buf_size / 2);
  twiddle::texture_4bpp(twiddle, texture, texture_width, texture_height);

  FILE * out = fopen(output_file_path, "w");
  if (out == NULL) {
    perror("fopen(w)");
    return -1;
  }
  //fwrite((void *)texture, texture_buf_size, 1, out);
  fwrite((void *)twiddle, texture_buf_size / 2, 1, out);
  fclose(out);
}
