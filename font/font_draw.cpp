

uint32_t pixel_data(const uint32_t * dest, const uint8_t * glyph_bitmaps, const uint32_t size)
{
  const uint8_t temp[size];

  const uint32_t * buf = reinterpret_cast<const uint32_t *>(&glyph_bitmaps[0]);

  copy<uint32_t>(table, buf, size);

  return table_address;
}

uint32_t font_data(const uint32_t * buf, state& state)
{
  constexpr uint32_t font_offset          = 0;
  constexpr uint32_t glyphs_offset        = (sizeof (struct font));
  const     uint32_t glyph_bitmaps_offset = (sizeof (struct font)) + (sizeof (struct glyph)) * font->glyph_index;

  auto font          = reinterpret_cast<const font *>(&buf[font_offset / 4]);
  auto glyphs        = reinterpret_cast<const glyph *>(&buf[glyphs_offset / 4]);
  auto glyph_bitmaps = &(reinterpret_cast<const uint8_t *>(buf))[glyph_bitmaps_offset];

  for (uint32_t glyph_ix = 0; glyph_ix < font->glyph_index; glyph_ix++) {
    auto& glyph_bitmap = glyphs[glyph_ix].bitmap;

    auto bitmap = &glyph_bitmaps[glyph_bitmap.offset];
    // bitmap.pitch may be zero; bitmap.pitch is a multiple of 8 pixels
    SIZE__X(bitmap.pitch) | SIZE__Y(bitmap.rows);
  }
}
