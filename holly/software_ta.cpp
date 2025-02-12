#include "software_ta.hpp"

#if defined(__dreamcast__)
#include "sh7091/serial.hpp"
#define print__character serial::character
#define print__string serial::string
#define print__integer serial::integer<int32_t>
#define assert(b)                                                       \
  do {                                                                  \
    if (!(b)) {                                                         \
      print__string(__FILE__);                                          \
      print__character(':');                                            \
      print__integer(__LINE__, ' ');                                    \
      print__string(__func__);                                          \
      print__string(": assertion failed: ");                            \
      print__string(#b);                                                \
      print__character('\n');                                           \
      while (1);                                                        \
    }                                                                   \
  } while (0);
#else
#include <stdio.h>
#include <assert.h>
#endif

union i32_f {
  int32_t i;
  float f;
};

namespace para_type {
  constexpr int end_of_list = 0;
  constexpr int user_tile_clip = 1;
  constexpr int object_list_set = 2;
  constexpr int polygon_or_modifier_volume = 4;
  constexpr int sprite = 5;
  constexpr int vertex = 7;
};

namespace list_type {
  constexpr int opaque = 0;
  constexpr int opaque_modifier_volume = 1;
  constexpr int translucent = 2;
  constexpr int translucent_modifier_volume = 3;
  constexpr int punch_through = 4;
};

namespace col_type {
  constexpr int packed_color = 0;
  constexpr int floating_color = 1;
  constexpr int intensity_mode_1 = 2;
  constexpr int intensity_mode_2 = 3;
};

namespace object_list_data {
  constexpr int triangle_strip = (0b0 << 31);
  constexpr int triangle_array = (0b100 << 29);
  constexpr int quad_array = (0b101 << 29);
  constexpr int object_pointer_block_link = (0b111 << 29);
};

// if entry == 0; calculate ol_current from ol_base
struct tile_state {
  int8_t entry;
  int8_t current_list_type;
  int32_t ol_base;       // words
  int32_t ol_current;    // words
};

struct alloc {
  union {
    int opaque;                         // words
    int opaque_modifier_volume;         // words
    int translucent;                    // words
    int translucent_modifier_volume;    // words
    int punch_through;                  // words
  };
  int list_type[5];
};

struct ta_state {
  int8_t tile_x_num;
  int8_t tile_y_num;

  int8_t current_list_type;
  int8_t list_end;

  int32_t ol_base;
  int32_t ol_limit;
  int32_t param_base;
  int32_t param_limit;   // words
  int32_t param_current; // words

  int32_t next_opb;    // words
  int32_t entry;
  struct alloc alloc;
  struct tile_state tile[64 * 64];
};

static struct ta_state state;

static inline int alloc_ctrl_units(int n)
{
  return (n == 0) ? 0 : (1 << (n + 2)); // words
}

static inline void terminate_ta_tiles(int list_type, void * _dst)
{
  union i32_f * dst = (union i32_f *)_dst;
  for (int y = 0; y < state.tile_y_num; y++) {
    for (int x = 0; x < state.tile_x_num; x++) {
      struct tile_state * tile = &state.tile[y * 64 + x];
      assert(tile->current_list_type == list_type || tile->current_list_type == -1);

      // end of list
      assert(tile->ol_current >= state.ol_base && tile->ol_current < state.ol_limit);
      fprintf(stderr, "eol %d %d %08x\n", x, y, tile->ol_current * 4);
      dst[tile->ol_current].i = object_list_data::object_pointer_block_link | (1 << 28);

      tile->current_list_type = -1;
    }
  }
}

static inline void tile_ol_current_for_list(struct tile_state * tile, int list_type_ix)
{
  assert(list_type_ix >= 0 && list_type_ix <= 5);
  tile->ol_current = tile->ol_base + state.alloc.list_type[list_type_ix];
  { // removeme
    int32_t ix = tile - state.tile;
    int x = ix & 63;
    int y = ix >> 6;
    fprintf(stderr, "ol_current_for_list %d %d %08x\n", x, y, tile->ol_current * 4);
  }
}

static inline void flush_ta_tile(int list_type, int length, int shadow, int skip, struct tile_state * tile, void * _dst)
{
  union i32_f * dst = (union i32_f *)_dst;

  if (tile->current_list_type != list_type) {
    tile_ol_current_for_list(tile, list_type);
    tile->current_list_type = list_type;
  }

  if (!tile->entry)
    return;

  assert(length > 0);
  assert(length <= 16);

  int32_t ix = tile - state.tile;
  int x = ix & 63;
  int y = ix >> 6;

  fprintf(stderr, "flush_ta_tile %d %d %08x\n", x, y, tile->ol_current * 4);
  if ((tile->ol_current & 15) == 15) {
    fprintf(stderr, "overflow %d\n", (int32_t)(tile - state.tile));
    tile->entry = 0;
  }
  //assert((tile->ol_current & 15) != 15);

  dst[tile->ol_current].i = (object_list_data::triangle_array |
                             (length - 1) << 25 |
                             (shadow << 24) |
                             (skip << 21) |
                             state.param_current);
  fprintf(stderr, "l1 %d\n", (length - 1));
  fprintf(stderr, "oldta %d\n", object_list_data::triangle_array);
  fprintf(stderr, "param_current %d\n", state.param_current);
  fprintf(stderr, "dst tile ol_current %d %08x\n", tile->ol_current * 4, dst[tile->ol_current].i);

  tile->ol_current += 1;
  tile->entry = 0;
}

void flush_ta_tiles(int list_type, int length, int shadow, int skip, void * dst)
{
  for (int y = 0; y < state.tile_y_num; y++) {
    for (int x = 0; x < state.tile_x_num; x++) {
      struct tile_state * tile = &state.tile[y * 64 + x];
      flush_ta_tile(list_type, length, shadow, skip, tile, dst);
    }
  }
}

void software_ta_init(const struct ta_configuration * config)
{
  state.tile_x_num = config->tile_x_num;
  state.tile_y_num = config->tile_y_num;
  state.current_list_type = -1;
  state.list_end = 0;

  state.ol_base = config->ol_base >> 2;  // words
  state.ol_limit = config->ol_limit >> 2;  // words
  state.param_base = config->isp_base >> 2;  // words
  state.param_limit = config->isp_limit >> 2;  // words
  state.param_current = state.param_base; // words
  assert(state.param_limit <= 0x1fffff);

  state.next_opb = config->next_opb_init;

  int alloc_opaque                      = alloc_ctrl_units((config->alloc_ctrl >> 0 ) & 0b11);
  int alloc_opaque_modifier_volume      = alloc_ctrl_units((config->alloc_ctrl >> 4 ) & 0b11);
  int alloc_translucent                 = alloc_ctrl_units((config->alloc_ctrl >> 8 ) & 0b11);
  int alloc_translucent_modifier_volume = alloc_ctrl_units((config->alloc_ctrl >> 12) & 0b11);
  int alloc_punch_through               = alloc_ctrl_units((config->alloc_ctrl >> 16) & 0b11);

  // bytes
  int alloc_total_size = (alloc_opaque +
                          alloc_opaque_modifier_volume +
                          alloc_translucent +
                          alloc_translucent_modifier_volume +
                          alloc_punch_through);

  state.alloc.opaque = 0;
  state.alloc.opaque_modifier_volume = state.alloc.opaque + alloc_opaque;
  state.alloc.translucent = state.alloc.opaque_modifier_volume + alloc_opaque_modifier_volume;
  state.alloc.translucent_modifier_volume = state.alloc.translucent + alloc_translucent;
  state.alloc.punch_through = state.alloc.translucent_modifier_volume + alloc_translucent_modifier_volume;

  int ol_offset = config->ol_base >> 2; // words
  for (int y = 0; y < config->tile_y_num; y++) {
    for (int x = 0; x < config->tile_x_num; x++) {
      struct tile_state * tile = &state.tile[y * 64 + x];
      fprintf(stderr, "ol_offset %d %d %08x\n", x, y, ol_offset);
      tile->entry = 0;
      tile->ol_base = ol_offset;
      tile->ol_current = -1;
      tile->current_list_type = -1;
      ol_offset += alloc_total_size;
    }
  }
}

static inline int decode_skip(int texture, int offset, int _16bit_uv)
{
  if (texture == 0)
    return 0b001;

  int skip;
  int control_word = (offset << 1) | (_16bit_uv << 0);
  switch (control_word) {
  default: [[fallthrough]];
  case 0b00: skip = 0b011; break;
  case 0b01: skip = 0b010; break;
  case 0b10: skip = 0b100; break;
  case 0b11: skip = 0b011; break;
  }

  return skip;
}

/*
int modifier_volume(union i32_f * parameter,
                    int list_type,
                    int parameter_control_word,
                    void * dst)
{
  int skip = 0;
}
*/

static inline int32_t pack_floating_color(float a, float r, float g, float b)
{
  /* The TA converts each element of ARGB data into a fixed decimal value
     between 0.0 and 1.0, multiples the value by 255, and packs the result in a
     32-bit value.
  */

  if (a > 1.f) a = 1.f;
  if (a < 0.f) a = 0.f;
  if (r > 1.f) r = 1.f;
  if (r < 0.f) r = 0.f;
  if (g > 1.f) g = 1.f;
  if (g < 0.f) g = 0.f;
  if (b > 1.f) b = 1.f;
  if (b < 0.f) b = 0.f;

  int ai = (int)(a * 255.f);
  int ri = (int)(r * 255.f);
  int gi = (int)(g * 255.f);
  int bi = (int)(b * 255.f);

  return ((ai & 0xff) << 24) | ((ri & 0xff) << 16) | ((gi & 0xff) << 8) | ((bi & 0xff) << 0);
}

static inline int32_t pack_intensity_color(float a, float r, float g, float b, float intensity)
{
  /* Regarding alpha values, the TA converts the specified Face Color Alpha
     value into a fixed decimal value between 0.0 and 1.0, multiples the value
     by 255, and derives an 8-bit value.  Regarding RGB values, the TA converts
     the specified Face Color R/G/B value into a fixed decimal value between 0.0
     and 1.0, multiples the value by 255, converts the intensity value into a
     fixed decimal value between 0.0 and 1.0, multiplies the converted R/G/B
     value and the converted intensity value together, multiplies that result by
     255, and derives an 8-bit value for each of R, G, and B.  Finally, the TA
     packs each 8-bit value into a 32-bit value.
  */

  // this description is probably incorrect. multiplying by 255 twice does not
  // make sense.

  if (a > 1.f) a = 1.f;
  if (a < 0.f) a = 0.f;
  if (r > 1.f) r = 1.f;
  if (r < 0.f) r = 0.f;
  if (g > 1.f) g = 1.f;
  if (g < 0.f) g = 0.f;
  if (b > 1.f) b = 1.f;
  if (b < 0.f) b = 0.f;
  if (intensity > 1.f) intensity = 1.f;
  if (intensity < 0.f) intensity = 0.f;

  int ai = (int)(a * 255.f);
  int ri = (int)(r * intensity * 255.f);
  int gi = (int)(g * intensity * 255.f);
  int bi = (int)(b * intensity * 255.f);

  if (ri > 255) ri = 255;
  if (gi > 255) gi = 255;
  if (bi > 255) bi = 255;

  return ((ai & 0xff) << 24) | (ri << 16) | (gi << 8) | (bi << 0);
}

struct bounding_box {
  int min_x; // in tile units
  int min_y; // in tile units
  int max_x; // in tile units
  int max_y; // in tile units
};

static inline float min(float a, float b, float c)
{
  if (a < b)
    return (a < c) ? a : c;
  else
    return (b < c) ? b : c;
}

static inline float max(float a, float b, float c)
{
  if (a > b)
    return (a > c) ? a : c;
  else
    return (b > c) ? b : c;
}

static inline int floor(float f)
{
  return (int)f;
}

static inline int ceil(float f)
{
  int fi = (int)f;
  float fn = (float)fi;
  return fi + (fn < f);
}

static inline struct bounding_box calculate_bounding_box(float ax, float ay,
                                                         float bx, float by,
                                                         float cx, float cy)
{
  int min_x = floor(min(ax, bx, cx));
  int min_y = floor(min(ay, by, cy));
  int max_x = ceil(max(ax, bx, cx));
  int max_y = ceil(max(ay, by, cy));

  return (bounding_box){
    min_x >> 5, // round down
    min_y >> 5, // round down
    max_x >> 5, // round down
    max_y >> 5, // round down
  };
}

struct previous_vertex {
  float x;
  float y;
  float z;
  int32_t base_color_0;
  int32_t offset_color_0;
  int32_t base_color_1;
  int32_t offset_color_1;
};

int polygon(union i32_f * parameter,
            int list_type,
            int parameter_control_word,
            void * _dst)
{
  union i32_f * dst = (union i32_f *)_dst;

  assert(((parameter_control_word >> 27) & 1) == 0);
  int group_en     = (parameter_control_word >> 23) & 0b1;
  assert(((parameter_control_word >> 20) & 0b111) == 0);
  int strip_len    = (parameter_control_word >> 18) & 0b11;
  int user_clip    = (parameter_control_word >> 16) & 0b11;
  assert(((parameter_control_word >> 8) & 0xff) == 0);
  int shadow       = (parameter_control_word >> 7) & 0b1;
  int volume       = (parameter_control_word >> 6) & 0b1;
  int col_type     = (parameter_control_word >> 4) & 0b11;
  int texture      = (parameter_control_word >> 3) & 0b1;
  int offset       = (parameter_control_word >> 2) & 0b1;
  int gouraud      = (parameter_control_word >> 1) & 0b1;
  int _16bit_uv    = (parameter_control_word >> 0) & 0b1;

  fprintf(stderr, "sv %d %d\n", shadow, volume);
  assert(!volume || shadow); // for polygons, shadow and volume must be the same value

  int32_t isp_tsp_instruction_word = parameter[1].i;
  isp_tsp_instruction_word &= ~(0b1111 << 22);
  isp_tsp_instruction_word |= (parameter_control_word & 0b1111) << 22;

#define tsp_instruction_word_0 parameter[2].i
#define texture_control_word_0 parameter[3].i
#define tsp_instruction_word_1 parameter[4].i
#define texture_control_word_1 parameter[5].i

  static float face_color_a_0;
  static float face_color_r_0;
  static float face_color_g_0;
  static float face_color_b_0;
  static float face_offset_color_a;
  static float face_offset_color_r;
  static float face_offset_color_g;
  static float face_offset_color_b;
#define face_color_a_1 face_offset_color_a
#define face_color_r_1 face_offset_color_r
#define face_color_g_1 face_offset_color_g
#define face_color_b_1 face_offset_color_b

  int vertex_index = 8;
  if (col_type == col_type::intensity_mode_1) {
    if (offset || volume) {
      face_color_a_0 = parameter[8].f;
      face_color_r_0 = parameter[9].f;
      face_color_g_0 = parameter[10].f;
      face_color_b_0 = parameter[11].f;
      face_offset_color_a = parameter[12].f;
      face_offset_color_r = parameter[13].f;
      face_offset_color_g = parameter[14].f;
      face_offset_color_b = parameter[15].f;
      vertex_index = 16;
    } else {
      face_color_a_0 = parameter[4].f;
      face_color_r_0 = parameter[5].f;
      face_color_g_0 = parameter[6].f;
      face_color_b_0 = parameter[7].f;
    }
  }

  int skip = decode_skip(texture, offset, _16bit_uv);

  int param_index = state.param_current;

  struct previous_vertex previous_vertex[2];

  int color_control_word = volume << 1 | texture << 0;
  int vertex_length = 8 * (1 + (texture && (volume || col_type == col_type::floating_color)));

  int strip_index = 0;

  int32_t base_color_0;
  int32_t offset_color_0;
  int32_t base_color_1;
  int32_t offset_color_1;
  float x;
  float y;
  float z;

  while (true) {
    int parameter_control_word = parameter[vertex_index + 0].i;
    fprintf(stderr, "vi %d %08x\n", vertex_index, parameter_control_word);

    int para_type    = (parameter_control_word >> 29) & 0b111;
    assert(para_type == para_type::vertex);
    int end_of_strip = (parameter_control_word >> 28) & 0b1;

    x = parameter[vertex_index + 1].f;
    y = parameter[vertex_index + 2].f;
    z = parameter[vertex_index + 3].f;

    switch (col_type) {
    case col_type::packed_color:
      switch (color_control_word) {
      case 0b00: // one volume, non-textured
        base_color_0 = parameter[vertex_index + 6].i;
        break;
      case 0b01: // one volume, textured
        base_color_0 = parameter[vertex_index + 6].i;
        offset_color_0 = parameter[vertex_index + 7].i;
        break;
      case 0b10: // two volumes, non-textured
        base_color_0 = parameter[vertex_index + 4].i;
        base_color_1 = parameter[vertex_index + 5].i;
        break;
      case 0b11: // two volumes, textured
        base_color_0 = parameter[vertex_index + 6].i;
        offset_color_0 = parameter[vertex_index + 7].i;
        base_color_1 = parameter[vertex_index + 10].i;
        offset_color_1 = parameter[vertex_index + 11].i;
        break;
      }
      break;
    case col_type::floating_color:
      {
        float base_color_a;
        float base_color_r;
        float base_color_g;
        float base_color_b;
        float offset_color_a;
        float offset_color_r;
        float offset_color_g;
        float offset_color_b;
        switch (color_control_word) {
        case 0b00: // one volume, non-textured
          base_color_a = parameter[vertex_index + 4].f;
          base_color_r = parameter[vertex_index + 5].f;
          base_color_g = parameter[vertex_index + 6].f;
          base_color_b = parameter[vertex_index + 7].f;

          base_color_0 = pack_floating_color(base_color_a, base_color_r, base_color_g, base_color_b);
          break;
        case 0b01: // one volume, textured
          base_color_a = parameter[vertex_index + 8].f;
          base_color_r = parameter[vertex_index + 9].f;
          base_color_g = parameter[vertex_index + 10].f;
          base_color_b = parameter[vertex_index + 11].f;
          base_color_a = parameter[vertex_index + 12].f;
          base_color_r = parameter[vertex_index + 13].f;
          base_color_g = parameter[vertex_index + 14].f;
          base_color_b = parameter[vertex_index + 15].f;

          base_color_0 = pack_floating_color(base_color_a, base_color_r, base_color_g, base_color_b);
          offset_color_0 = pack_floating_color(offset_color_a, offset_color_r, offset_color_g, offset_color_b);
          break;
        case 0b10: // two volumes, non-textured
          assert(!"two volume non-textured floating color");
          break;
        case 0b11: // two volumes, textured
          assert(!"two volume textured floating color");
          break;
        }
      }
      break;
    case col_type::intensity_mode_1: [[fallthrough]];
    case col_type::intensity_mode_2:
      {
        float base_intensity_0;
        float base_intensity_1;
        float offset_intensity_0;
        float offset_intensity_1;
        switch (color_control_word) {
        case 0b00: // one volume, non-textured
          base_intensity_0 = parameter[vertex_index + 6].f;
          base_color_0 = pack_intensity_color(face_color_a_0, face_color_r_0, face_color_g_0, face_color_b_0, base_intensity_0);
          break;
        case 0b01: // one volume, textured
          base_intensity_0 = parameter[vertex_index + 6].f;
          offset_intensity_0 = parameter[vertex_index + 7].f;
          base_color_0 = pack_intensity_color(face_color_a_0, face_color_r_0, face_color_g_0, face_color_b_0, base_intensity_0);
          offset_color_0 = pack_intensity_color(face_offset_color_a, face_offset_color_r, face_offset_color_g, face_offset_color_b, offset_intensity_0);
          break;
        case 0b10: // two volumes, non-textured
          base_intensity_0 = parameter[vertex_index + 4].f;
          base_intensity_1 = parameter[vertex_index + 5].f;
          base_color_0 = pack_intensity_color(face_color_a_0, face_color_r_0, face_color_g_0, face_color_b_0, base_intensity_0);
          base_color_1 = pack_intensity_color(face_color_a_1, face_color_r_1, face_color_g_1, face_color_b_1, base_intensity_0);
          break;
        case 0b11: // two volumes, textured
          base_intensity_0 = parameter[vertex_index + 6].f;
          offset_intensity_0 = parameter[vertex_index + 7].f;
          base_color_0 = pack_intensity_color(face_color_a_0, face_color_r_0, face_color_g_0, face_color_b_0, base_intensity_0);
          offset_color_0 = pack_intensity_color(face_offset_color_a, face_offset_color_r, face_offset_color_g, face_offset_color_b, offset_intensity_0);

          base_intensity_1 = parameter[vertex_index + 10].f;
          offset_intensity_1 = parameter[vertex_index + 11].f;
          base_color_1 = pack_intensity_color(face_color_a_1, face_color_r_1, face_color_g_1, face_color_b_1, base_intensity_1);
          offset_color_1 = pack_intensity_color(face_offset_color_a, face_offset_color_r, face_offset_color_g, face_offset_color_b, offset_intensity_1);
          break;
        }
      } // switch col_type
    }

    if (strip_index >= 2) {
      // write an entire triangle to params:
      dst[param_index + 0].i = isp_tsp_instruction_word;
      dst[param_index + 1].i = tsp_instruction_word_0;
      dst[param_index + 2].i = texture_control_word_0;
      param_index += 3;

      dst[param_index + 0].f = previous_vertex[1].x;
      dst[param_index + 1].f = previous_vertex[1].y;
      dst[param_index + 2].f = previous_vertex[1].z;
      dst[param_index + 3].i = previous_vertex[1].base_color_0;
      assert(skip + 3 == 4);
      param_index += 4;

      dst[param_index + 0].f = previous_vertex[0].x;
      dst[param_index + 1].f = previous_vertex[0].y;
      dst[param_index + 2].f = previous_vertex[0].z;
      dst[param_index + 3].i = previous_vertex[0].base_color_0;
      assert(skip + 3 == 4);
      param_index += 4;

      dst[param_index + 0].f = x;
      dst[param_index + 1].f = y;
      dst[param_index + 2].f = z;
      dst[param_index + 3].i = base_color_0;
      assert(skip + 3 == 4);
      param_index += 4;
    }

    if (strip_index >= 2) {
      // find new tiles to includes

      // 2: {v1, v0}
      // 3: {v1, v2}
      // 4: {v3, v2}
      // 5: {v3, v4}


      /*
       1 p0 B         p0 B---D _
           / \            \ /
     0 p1 A---C 2 _    p1  C
       */

      assert(texture == 0);

      // calculate bounding box
      struct bounding_box bb = calculate_bounding_box(x, y,
                                                      previous_vertex[0].x, previous_vertex[0].y,
                                                      previous_vertex[1].x, previous_vertex[1].y);
      fprintf(stderr, "bb: min %d %d max %d %d\n", bb.min_x, bb.min_y, bb.max_x, bb.max_y);
      for (int ty = bb.min_y; ty <= bb.max_y; ty++) {
        for (int tx = bb.min_x; tx <= bb.max_x; tx++) {
          int tile_ix = ty * 64 + tx;
          struct tile_state * tile = &state.tile[tile_ix];
          tile->entry = 1;
        }
      }
    }

    // check for possible end of strip
    strip_index += 1;
    if (end_of_strip) {
      assert(strip_index >= 2);
      int num_triangles = strip_index - 2;
      assert(num_triangles >= 0);
      flush_ta_tiles(list_type, num_triangles, shadow, skip, dst);
      state.param_current = param_index;
      strip_index = 0;
    }

    // next_vertex (possible end of polygon array)
    vertex_index += vertex_length;
    parameter_control_word = parameter[vertex_index + 0].i;
    para_type    = (parameter_control_word >> 29) & 0b111;
    if (para_type != para_type::vertex) {
      assert(end_of_strip);
      break;
    }

    // write previous vertex parameters to triangle strip array
    // 0: 1 & (strip_index & 1) → 1
    // 1: 2 & (strip_index & 1) → 0
    // 2: 3 & (strip_index & 1) → 1
    // 3: 4 & (strip_index & 1) → 0
    // 4: 5 & (strip_index & 1) → 1
    // 5: 6 & (strip_index & 1) → 0
    previous_vertex[strip_index & 1].x = x;
    previous_vertex[strip_index & 1].y = y;
    previous_vertex[strip_index & 1].z = z;
    previous_vertex[strip_index & 1].base_color_0 = base_color_0;
    previous_vertex[strip_index & 1].offset_color_0 = offset_color_0;
    previous_vertex[strip_index & 1].base_color_1 = base_color_1;
    previous_vertex[strip_index & 1].offset_color_1 = offset_color_1;
  }

  return vertex_index << 2;
}

void software_ta_transfer(void * src, int32_t src_size,
                          void * dst)
{
  int32_t src_offset = 0;
  while (src_offset < src_size) {
    fprintf(stderr, "src_offset: %08x\n", src_offset);
    union i32_f * parameter = (union i32_f *)(((int8_t *)src) + src_offset);
    int32_t parameter_control_word = parameter[0].i;
    int para_type    = (parameter_control_word >> 29) & 0b111;
    int list_type    = (parameter_control_word >> 24) & 0b111;

    switch (para_type) {
    case para_type::end_of_list:
      assert(state.current_list_type != -1);
      state.list_end |= (1 << state.current_list_type);
      terminate_ta_tiles(state.current_list_type, dst);
      state.current_list_type = -1;
      src_offset += 32;
      break;
    case para_type::user_tile_clip:
      src_offset += 32;
      assert(!"user tile clip");
      break;
    case para_type::object_list_set:
      src_offset += 32;
      assert(!"object list set");
      break;
    case para_type::polygon_or_modifier_volume:
      fprintf(stderr, "%d %d\n", state.current_list_type, list_type);
      fflush(stdout);
      assert(state.current_list_type == -1 || state.current_list_type == list_type);
      state.current_list_type = list_type;
      if (list_type == list_type::opaque_modifier_volume || list_type == list_type::translucent_modifier_volume) {
        assert(!"modifier_volume");
        //src_offset += modifier_volume(parameter, list_type, parameter_control_word, dst);
      } else {
        src_offset += polygon(parameter, list_type, parameter_control_word, dst);
      }
      break;
    case para_type::sprite:
      assert(!"sprite");
      break;
    case para_type::vertex:
      assert(!"vertex parameter with no global");
      break;
    default:
      assert(!"invalid para_type");
      break;
    }
  }

  assert(src_offset == src_size);
}
