namespace maple {

namespace display {

namespace vmu {
constexpr int32_t width = 48;
constexpr int32_t height = 32;
constexpr int32_t pixels_per_byte = 8;
constexpr int32_t framebuffer_size = width * height / pixels_per_byte;
}

struct font_renderer {
  // 6x8 px font assumed
  uint8_t const * const font;
  uint8_t fb[vmu::framebuffer_size];

  constexpr font_renderer(uint8_t const * const font)
    : font(font)
  { }

  constexpr inline void glyph(uint8_t c, int x, int y)
  {
    int y_ix = 186 - (y * 6 * 8);
    for (int i = 0; i < 8; i++) {
      switch (x) {
      case 0:
	fb[y_ix - i * 6 + 5] = font[(c - ' ') * 8 + i];
	break;
      case 1:
	fb[y_ix - i * 6 + 5] |= (font[(c - ' ') * 8 + i] & 0b11) << 6;
	fb[y_ix - i * 6 + 4] = font[(c - ' ') * 8 + i] >> 2; // 0b1111
	break;
      case 2:
	fb[y_ix - i * 6 + 4] |= (font[(c - ' ') * 8 + i] & 0b1111) << 4;
	fb[y_ix - i * 6 + 3] = font[(c - ' ') * 8 + i] >> 4; // 0b11
	break;
      case 3:
	fb[y_ix - i * 6 + 3] |= font[(c - ' ') * 8 + i] << 2;
	break;
      case 4:
	fb[y_ix - i * 6 + 2] = font[(c - ' ') * 8 + i];
	break;
      case 5:
	fb[y_ix - i * 6 + 2] |= (font[(c - ' ') * 8 + i] & 0b11) << 6;
	fb[y_ix - i * 6 + 1] = font[(c - ' ') * 8 + i] >> 2; // 0b1111
	break;
      case 6:
	fb[y_ix - i * 6 + 1] |= (font[(c - ' ') * 8 + i] & 0b1111) << 4;
	fb[y_ix - i * 6 + 0] = font[(c - ' ') * 8 + i] >> 4; // 0b11
	break;
      case 7:
	fb[y_ix - i * 6 + 0] |= font[(c - ' ') * 8 + i] << 2;
	break;
      }
    }
  }

};

} // namespace display
} // namespace maple
