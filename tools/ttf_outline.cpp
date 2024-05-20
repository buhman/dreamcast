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

template< class T >
constexpr T byteswap(const T n)
{
  if (std::endian::native != _target_endian) {
    return std::byteswap<T>(n);
  } else {
    return n;
  }
}

int32_t
load_outline_char_bitmap_rect(const FT_Face face,
                              const FT_Int32 load_flags,
			      const FT_ULong char_code,
			      struct rect& rect)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, load_flags);
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
                  const FT_Int32 load_flags,
                  const FT_Render_Mode render_mode,
		  const uint32_t bits_per_pixel,
                  const FT_ULong char_code,
                  glyph * glyph,
                  uint8_t * texture,
		  uint32_t texture_width,
		  struct rect& rect)
{
  FT_Error error;
  FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);

  error = FT_Load_Glyph(face, glyph_index, load_flags);
  if (error) {
    std::cerr << "FT_Load_Glyph " << FT_Error_String(error) << '\n';
    return -1;
  }

  //std::cerr << "size " << face->glyph->bitmap.rows << ' ' << face->glyph->bitmap.width << '\n';

  //assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

  error = FT_Render_Glyph(face->glyph, render_mode);
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

  assert(bits_per_pixel == 8 || bits_per_pixel == 4 || bits_per_pixel == 2 || bits_per_pixel == 1);
  const uint32_t pixels_per_byte = 8 / bits_per_pixel;
  const uint32_t texture_stride = texture_width / pixels_per_byte;
  std::cerr << "pixels per byte: " << pixels_per_byte << '\n';
  std::cerr << "texture stride: " << texture_stride << '\n';

  for (uint32_t y = 0; y < rect.height; y++) {
    for (uint32_t x = 0; x < rect.width; x++) {
      const uint32_t texture_ix = (rect.y + y) * texture_stride + (rect.x + x) / pixels_per_byte;
      const uint32_t texture_ix_mod = (rect.x + x) % pixels_per_byte;
      assert(texture_ix < max_texture_size);

      uint8_t level;

      //std::cerr << "rxy " << rect.x << ' ' << rect.y << '\n';
      //std::cerr << "rwh " << rect.width << ' ' << rect.height << '\n';

      //std::cerr << "pixel_mode " << (int)face->glyph->bitmap.pixel_mode << '\n';
      switch (face->glyph->bitmap.pixel_mode) {
      case FT_PIXEL_MODE_MONO:
        // [num_grays] is only used with FT_PIXEL_MODE_GRAY; it gives the number
        // of gray levels used in the bitmap.
        level = (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + (x / 8)] >> (7 - (x % 8))) & 1;
        break;
      case FT_PIXEL_MODE_GRAY:
        assert(face->glyph->bitmap.num_grays == 256);
        //std::cerr << "num_grays " << face->glyph->bitmap.num_grays << '\n';
        level = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
	level >>= (8 - bits_per_pixel);
        break;
      default:
        assert(false);
        break;
      }
      texture[texture_ix] |= level << (bits_per_pixel * texture_ix_mod);
    }
  }

  glyph_bitmap& bitmap = glyph->bitmap;
  bitmap.x = byteswap<uint16_t>(rect.x);
  bitmap.y = byteswap<uint16_t>(rect.y);
  bitmap.width = byteswap<uint16_t>(rect.width);
  bitmap.height = byteswap<uint16_t>(rect.height);

  glyph_metrics& metrics = glyph->metrics;
  metrics.horiBearingX = byteswap<int32_t>(face->glyph->metrics.horiBearingX);
  metrics.horiBearingY = byteswap<int32_t>(face->glyph->metrics.horiBearingY);
  metrics.horiAdvance = byteswap<int32_t>(face->glyph->metrics.horiAdvance);

  return 0;
}

enum {
  start_hex = 1,
  end_hex = 2,
  pixel_size = 3,
  monochrome_out = 4,
  target_endian = 5,
  font_file_path = 6,
  output_file_path = 7,
  argv_length = 8
};

struct window_curve_ix
load_all_positions(const FT_Face face,
                   bool monochrome,
		   const uint32_t start,
		   const uint32_t end,
		   glyph * glyphs,
		   uint32_t * texture
		   )
{
  const uint32_t num_glyphs = (end - start) + 1;
  struct rect rects[num_glyphs];

  FT_Int32 load_flags;
  FT_Render_Mode render_mode;
  if (monochrome) {
    load_flags = FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
    render_mode = FT_RENDER_MODE_MONO;
  } else {
    load_flags = FT_LOAD_DEFAULT;
    render_mode = FT_RENDER_MODE_NORMAL;
  }

  // first, load all rectangles
  for (uint32_t char_code = start; char_code <= end; char_code++) {
    load_outline_char_bitmap_rect(face,
                                  load_flags,
                                  char_code,
                                  rects[char_code - start]);
  }

  // calculate a 2-dimensional packing for the rectangles
  auto window_curve_ix = pack_all(rects, num_glyphs);

  const uint32_t bits_per_pixel = monochrome ? 1 : 8;

  // render all of the glyphs to the texture;
  for (uint32_t i = 0; i < num_glyphs; i++) {
    const uint32_t char_code = rects[i].char_code;
    int32_t err = load_outline_char(face,
                                    load_flags,
                                    render_mode,
				    bits_per_pixel,
				    char_code,
				    &glyphs[char_code - start],
				    reinterpret_cast<uint8_t *>(texture),
				    window_curve_ix.window.width,
				    rects[i]);
    if (err < 0) assert(false);
  }

  return window_curve_ix;
}

int main(int argc, char *argv[])
{
  FT_Library library;
  FT_Face face;
  FT_Error error;

  if (argc != argv_length) {
    std::cerr << "usage: " << argv[0] << " [start-hex] [end-hex] [pixel-size] [monochrome-out] [target-endian] [font-file-path] [output-file-path]\n\n";
    std::cerr << "ex. 1: " << argv[0] << " 3000 30ff 30 0 little ipagp.ttf font.bin\n";
    std::cerr << "ex. 2: " << argv[0] << " 20 7f 30 1 big DejaVuSans.ttf font.bin\n";
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
  std::stringstream ss4;
  int monochrome;
  ss4 << std::dec << argv[monochrome_out];
  ss4 >> monochrome;
  assert(monochrome == 0 || monochrome == 1);
  std::cerr << "monochrome: " << monochrome << '\n';

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

  auto window_curve_ix = load_all_positions(face, monochrome, start, end, glyphs, texture);

  uint32_t texture_stride;
  uint32_t texture_size;
  if (monochrome) {
    texture_stride = window_curve_ix.window.width / 8;
    texture_size = window_curve_ix.window.width * window_curve_ix.window.height / 8;
  } else {
    texture_stride = window_curve_ix.window.width;
    texture_size = window_curve_ix.window.width * window_curve_ix.window.height;
  }

  font font;
  font.first_char_code = byteswap<uint32_t>(start);
  font.last_char_code = byteswap<uint32_t>(end);
  font.face_metrics.height = byteswap<int32_t>(face->size->metrics.height);
  font.face_metrics.max_advance = byteswap<int32_t>(face->size->metrics.max_advance);
  font.glyph_count = byteswap<uint16_t>(num_glyphs);
  font.texture_stride = byteswap<uint16_t>(texture_stride);
  font.texture_width = byteswap<uint16_t>(window_curve_ix.window.width);
  font.texture_height = byteswap<uint16_t>(window_curve_ix.window.height);
  font.texture_size = byteswap<uint32_t>(texture_size);
  font.max_z_curve_ix = byteswap<uint32_t>(window_curve_ix.max_z_curve_ix);

  std::cerr << "start: 0x" << std::hex << start << '\n';
  std::cerr << "end: 0x"   << std::hex << end   << '\n';
  std::cerr << "texture_stride: "  << std::dec << texture_stride   << '\n';
  std::cerr << "texture_width: "  << std::dec << window_curve_ix.window.width << '\n';
  std::cerr << "texture_height: " << std::dec << window_curve_ix.window.height  << '\n';
  std::cerr << "texture_size: " << std::dec << texture_size << '\n';
  std::cerr << "max_z_curve_ix: " << std::dec << window_curve_ix.max_z_curve_ix << '\n';

  FILE * out = fopen(argv[output_file_path], "w");
  if (out == NULL) {
    perror("fopen(w)");
    return -1;
  }

  fwrite(reinterpret_cast<void*>(&font), (sizeof (font)), 1, out);
  fwrite(reinterpret_cast<void*>(&glyphs[0]), (sizeof (glyph)), num_glyphs, out);
  fwrite(reinterpret_cast<void*>(&texture[0]), (sizeof (uint8_t)), texture_size, out);

  fclose(out);
}
