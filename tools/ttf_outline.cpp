#include <bit>
#include <sstream>
#include <iostream>

#include <cassert>
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../font/font.hpp"
#include "rect.hpp"
#include "2d_pack.hpp"
#include "../twiddle.hpp"

std::endian _target_endian;

constexpr uint32_t max_texture_dim = 1024;
constexpr uint32_t max_texture_size = max_texture_dim * max_texture_dim;

uint32_t byteswap(const uint32_t n)
{
  if (std::endian::native != _target_endian) {
    return std::byteswap(n);
  } else {
    return n;
  }
}

int32_t
load_outline_char_bitmap_rect(const FT_Face face,
			      const FT_ULong char_code,
			      struct rect& rect)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    std::cerr << "FT_Load_Glyph " << FT_Error_String(error) << '\n';
    return -1;
  }

  rect.char_code = char_code;
  rect.height = face->glyph->bitmap.rows;
  rect.width = face->glyph->bitmap.width;
  rect.x = -1;
  rect.y = -1;

  return 0;
}

int32_t
load_outline_char(const FT_Face face,
                  const FT_ULong char_code,
                  glyph * glyph,
                  uint8_t * texture,
		  struct rect& rect)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    std::cerr << "FT_Load_Glyph " << FT_Error_String(error) << '\n';
    return -1;
  }

  //std::cerr << "size " << face->glyph->bitmap.rows << ' ' << face->glyph->bitmap.width << '\n';

  //assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

  error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
  if (error) {
    std::cerr << "FT_Render_Glyph " << FT_Error_String(error) << '\n';
    return -1;
  }

  if (!(face->glyph->bitmap.pitch > 0)) {
    assert(face->glyph->bitmap.width == 0);
    assert(face->glyph->bitmap.rows == 0);
  }

  assert(face->glyph->bitmap.width == rect.width);
  assert(face->glyph->bitmap.rows == rect.height);

  //std::cerr << "num_grays " << face->glyph->bitmap.num_grays << '\n';
  switch (face->glyph->bitmap.num_grays) {
  case 2:
    assert(false);
    break;
  case 256:
    //std::cerr << "rxy " << rect.x << ' ' << rect.y << '\n';
    //std::cerr << "rwh " << rect.width << ' ' << rect.height << '\n';

    for (uint32_t y = 0; y < rect.height; y++) {
      for (uint32_t x = 0; x < rect.width; x++) {
	uint32_t texture_ix = (rect.y + y) * max_texture_dim + (rect.x + x);
	assert(texture_ix < max_texture_size);
	texture[texture_ix] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
      }
    }

    break;
  default:
    assert(face->glyph->bitmap.num_grays == -1);
  }

  glyph_bitmap& bitmap = glyph->bitmap;
  bitmap.x = byteswap(rect.x);
  bitmap.y = byteswap(rect.y);
  bitmap.width = byteswap(rect.width);
  bitmap.height = byteswap(rect.height);

  glyph_metrics& metrics = glyph->metrics;
  metrics.horiBearingX = byteswap(face->glyph->metrics.horiBearingX);
  metrics.horiBearingY = byteswap(face->glyph->metrics.horiBearingY);
  metrics.horiAdvance = byteswap(face->glyph->metrics.horiAdvance);

  return 0;
}

enum {
  start_hex = 1,
  end_hex = 2,
  pixel_size = 3,
  target_endian = 4,
  font_file_path = 5,
  output_file_path = 6,
  argv_length = 7
};

struct window_curve_ix
load_all_positions(const FT_Face face,
		   const uint32_t start,
		   const uint32_t end,
		   glyph * glyphs,
		   uint32_t * texture
		   )
{
  const uint32_t num_glyphs = (end - start) + 1;
  struct rect rects[num_glyphs];

  uint8_t temp[max_texture_size];

  // first, load all rectangles
  for (uint32_t char_code = start; char_code <= end; char_code++) {
    load_outline_char_bitmap_rect(face, char_code, rects[char_code - start]);
  }

  // calculate a 2-dimensional packing for the rectangles
  auto window_curve_ix = pack_all(rects, num_glyphs);

  // asdf
  for (uint32_t i = 0; i < num_glyphs; i++) {
    const uint32_t char_code = rects[i].char_code;
    int32_t err = load_outline_char(face,
				    char_code,
				    &glyphs[char_code - start],
				    temp,
				    rects[i]);
    if (err < 0) assert(false);
  }

  twiddle::texture2<8>(texture, temp,
		       window_curve_ix.window.width,
		       window_curve_ix.window.height,
		       max_texture_dim);

  return window_curve_ix;
}

int main(int argc, char *argv[])
{
  FT_Library library;
  FT_Face face;
  FT_Error error;

  if (argc != argv_length) {
    std::cerr << "usage: " << argv[0] << " [start-hex] [end-hex] [pixel-size] [target-endian] [font-file-path] [output-file-path]\n\n";
    std::cerr << "ex. 1: " << argv[0] << " 3000 30ff 30 little ipagp.ttf font.bin\n";
    std::cerr << "ex. 2: " << argv[0] << " 20 7f 30 big DejaVuSans.ttf font.bin\n";
    return -1;
  }

  error = FT_Init_FreeType(&library);
  if (error) {
    std::cerr << "FT_Init_FreeType\n";
    return -1;
  }

  error = FT_New_Face(library, argv[font_file_path], 0, &face);
  if (error) {
    std::cerr << "FT_New_Face\n";
    return -1;
  }

  std::stringstream ss3;
  int font_size;
  ss3 << std::dec << argv[pixel_size];
  ss3 >> font_size;
  std::cerr << "font_size: " << font_size << '\n';

  error = FT_Set_Pixel_Sizes(face, 0, font_size);
  if (error) {
    std::cerr << "FT_Set_Pixel_Sizes: " << FT_Error_String(error) << error << '\n';
    return -1;
  }

  if (std::string(argv[target_endian]).compare("little") == 0) {
    _target_endian = std::endian::little;
  } else if (std::string(argv[target_endian]).compare("big") == 0) {
    _target_endian = std::endian::big;
  } else {
    std::cerr << "unknown endian: " << argv[target_endian] << '\n';
    std::cerr << "expected one of: big, little\n";
    return -1;
  }

  uint32_t start;
  uint32_t end;

  std::stringstream ss1;
  ss1 << std::hex << argv[start_hex];
  ss1 >> start;
  std::stringstream ss2;
  ss2 << std::hex << argv[end_hex];
  ss2 >> end;

  uint32_t num_glyphs = (end - start) + 1;
  glyph glyphs[num_glyphs];
  uint32_t texture[max_texture_size / 4];
  memset(texture, 0x00, max_texture_size);

  auto window_curve_ix = load_all_positions(face, start, end, glyphs, texture);

  font font;
  font.first_char_code = byteswap(start);
  font.glyph_count = byteswap(num_glyphs);
  font.glyph_height = byteswap(face->size->metrics.height);
  font.texture_width = byteswap(window_curve_ix.window.width);
  font.texture_height = byteswap(window_curve_ix.window.height);
  font.max_z_curve_ix = byteswap(window_curve_ix.max_z_curve_ix);

  std::cerr << "start: 0x" << std::hex << start << '\n';
  std::cerr << "end: 0x"   << std::hex << end   << '\n';
  std::cerr << "texture_width: "  << std::dec << window_curve_ix.window.width   << '\n';
  std::cerr << "texture_height: " << std::dec << window_curve_ix.window.height  << '\n';
  std::cerr << "max_z_curve_ix: " << std::dec << window_curve_ix.max_z_curve_ix << '\n';

  FILE * out = fopen(argv[output_file_path], "w");
  if (out == NULL) {
    perror("fopen(w)");
    return -1;
  }

  fwrite(reinterpret_cast<void*>(&font), (sizeof (font)), 1, out);
  fwrite(reinterpret_cast<void*>(&glyphs[0]), (sizeof (glyph)), num_glyphs, out);
  fwrite(reinterpret_cast<void*>(&texture[0]), (sizeof (uint8_t)), window_curve_ix.max_z_curve_ix + 1, out);

  fclose(out);
}
