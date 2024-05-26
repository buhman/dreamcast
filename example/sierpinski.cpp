#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/video_output.hpp"
#include "memorymap.hpp"
#include "twiddle.hpp"
#include "sh7091/store_queue.hpp"

#include "math/vec2.hpp"
#include "wolf2.hpp"
#include "strawberry.hpp"

/*
            a

          s   q

        c   r   b

saq
qbr
rcs
*/

using vec2 = vec<2, float>;

uint32_t tris;
uint32_t max_tris;

void triangle(vec2 a, vec2 b, vec2 c, const uint32_t base_color)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::shadow
                                        | obj_control::volume::polygon::with_two_volumes
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(1024)
                                      | tsp_instruction_word::texture_v_size::from_int(1024);

  const uint32_t texture_address_0 = texture_memory_alloc::texture.start + 1024 * 1024 * 0;
  const uint32_t texture_control_word_0 = texture_control_word::pixel_format::_8bpp_palette
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address_0 / 8)
                                        | texture_control_word::palette_selector8(0);

  const uint32_t texture_address_1 = texture_memory_alloc::texture.start + 1024 * 1024 * 1;
  const uint32_t texture_control_word_1 = texture_control_word::pixel_format::_8bpp_palette
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address_1 / 8)
                                        | texture_control_word::palette_selector8(1);

  *reinterpret_cast<ta_global_parameter::polygon_type_3 *>(store_queue) =
    ta_global_parameter::polygon_type_3(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,   // tsp_instruction_word_0
                                        texture_control_word_0, // texture_control_word_0
                                        tsp_instruction_word,   // tsp_instruction_word_1
                                        texture_control_word_1, // texture_control_word_1
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_11 *>(store_queue) =
    ta_vertex_parameter::polygon_type_11(polygon_vertex_parameter_control_word(false),
                                         a.x, a.y, 2.0f,  // x, y, z
                                         0.5f, 1.0f,      // u, v
                                         base_color, 0,
                                         0.5f, 1.0f,      // u, v
                                         base_color, 0);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_11 *>(store_queue) =
    ta_vertex_parameter::polygon_type_11(polygon_vertex_parameter_control_word(false),
                                         b.x, b.y, 2.0f,  // x, y, z
                                         0.0f, 0.11111975f, // u, v
                                         base_color, 0,
					 0.0f, 0.11111975f, // u, v
                                         base_color, 0);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_11 *>(store_queue) =
    ta_vertex_parameter::polygon_type_11(polygon_vertex_parameter_control_word(true), // end_of_strip
                                         c.x, c.y, 2.0f,  // x, y, z
                                         1.0f, 0.11111975f, // u, v
                                         base_color, 0,
					 1.0f, 0.11111975f, // u, v
                                         base_color, 0);
  sq_transfer_64byte(ta_fifo_polygon_converter);
}

vec2 midpoint(vec2 a, vec2 b)
{
  return {(a.x + b.x) / 2.f, (a.y + b.y) / 2.f};
}

constexpr uint32_t yellow = 0xfff0f000;
constexpr uint32_t blue   = 0xff0000ff;
constexpr uint32_t green  = 0xff00ff00;

void shadow_volume(vec2 a, vec2 b, vec2 c)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque_modifier_volume;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  *reinterpret_cast<ta_global_parameter::modifier_volume *>(store_queue) =
    ta_global_parameter::modifier_volume(parameter_control_word,
					 isp_tsp_instruction_word
					 );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  // top
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 1.0f,
					 b.x, b.y, 1.0f,
					 c.x, c.y, 1.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  //   at--ab
  //     \/_\	  .
  // ct  bt  bb
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 1.0f,
					 a.x, a.y, 3.0f,
					 b.x, b.y, 1.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 3.0f,
					 b.x, b.y, 3.0f,
					 b.x, b.y, 1.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  //  ab--at
  //   | / |
  //  cb--ct  bt
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 1.0f,
					 c.x, c.y, 3.0f,
					 a.x, a.y, 3.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 1.0f,
					 c.x, c.y, 1.0f,
					 c.x, c.y, 3.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  //     at
  //
  //   ct--bt
  //   | \ |
  //   cb--bb
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 c.x, c.y, 1.0f,
					 b.x, b.y, 1.0f,
					 b.x, b.y, 3.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 c.x, c.y, 1.0f,
					 b.x, b.y, 3.0f,
					 c.x, c.y, 3.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);


  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
					     | para_control::list_type::opaque_modifier_volume
					     | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
					       | isp_tsp_instruction_word::culling_mode::no_culling;
  *reinterpret_cast<ta_global_parameter::modifier_volume *>(store_queue) =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
					 last_isp_tsp_instruction_word
					 );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  // bottom
  *reinterpret_cast<ta_vertex_parameter::modifier_volume *>(store_queue) =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					 a.x, a.y, 3.0f,
					 b.x, b.y, 3.0f,
					 c.x, c.y, 3.0f);
  sq_transfer_64byte(ta_fifo_polygon_converter);
}

void subdivide(vec2 a, vec2 b, vec2 c, int depth)
{
  vec2 q = midpoint(a, b);
  vec2 r = midpoint(b, c);
  vec2 s = midpoint(c, a);
  //triangle(s, a, q, green);
  //triangle(q, b, r, green);
  //triangle(r, c, s, green);

  if (depth <= 0)
    return;

  subdivide(s, a, q, depth - 1);
  subdivide(q, b, r, depth - 1);
  subdivide(r, c, s, depth - 1);

  if (tris++ > max_tris)
    return;

  shadow_volume(q, r, s);
}

vec2 transform(vec2 v, float theta)
{
  v.x -= 320.f;
  v.y -= 240.f;
  float x = v.x * __builtin_cosf(theta) - v.y * __builtin_sinf(theta);
  float y = v.x * __builtin_sinf(theta) + v.y * __builtin_cosf(theta);
  return {x + 320.f, y + 240.f};
}

void copy_texture(const uint8_t * src, volatile uint32_t * texture)
{
  constexpr uint32_t size = 1024 * 1024 / 4;
  uint32_t temp[size];
  twiddle::texture(reinterpret_cast<uint8_t *>(temp), src, 1024, 1024);
  for (uint32_t i = 0; i < size; i++) {
    texture[i] = temp[i];
  }
}

void copy_palette(const uint8_t * src, const uint32_t palette)
{
  for (uint32_t i = 0; i < 256; i++) {
    uint8_t a = 255;
    uint8_t r = src[i * 3 + 0];
    uint8_t g = src[i * 3 + 1];
    uint8_t b = src[i * 3 + 2];
    holly.PALETTE_RAM[palette * 256 + i] = (a << 24) | (r << 16) | (g << 8) | (b << 0);
  }
}

void copy_textures_palettes()
{
  auto texture = reinterpret_cast<volatile uint32_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);

  auto wolf_src = reinterpret_cast<const uint8_t *>(&_binary_wolf2_data_start);
  auto wolf_pal_src = reinterpret_cast<const uint8_t *>(&_binary_wolf2_data_pal_start);
  auto wolf_texture = &texture[1024 * 1024 / 4 * 1];
  copy_texture(wolf_src, wolf_texture);
  copy_palette(wolf_pal_src, 1);

  auto strawberry_src = reinterpret_cast<const uint8_t *>(&_binary_strawberry_data_start);
  auto strawberry_pal_src = reinterpret_cast<const uint8_t *>(&_binary_strawberry_data_pal_start);
  auto strawberry_texture = &texture[1024 * 1024 / 4 * 0];
  copy_texture(strawberry_src, strawberry_texture);
  copy_palette(strawberry_pal_src, 0);
}

void main()
{
  video_output::set_mode_vga();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 16 * 4
                                       , .translucent = 0
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  region_array2(640 / 32, 480 / 32, opb_size);
  background_parameter(0xff0000ff);
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb8888;
  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;
  copy_textures_palettes();

  uint32_t frame_ix = 0;
  float theta = 0;
  constexpr float half_degree = 0.01745329f / 2.f;

  constexpr uint32_t reset_tris = 364 + 1;

  max_tris = 0;
  uint32_t frame = 0;
  while (true) {
    tris = 0;

    vec2 a = transform({320.000f,   5.f}, theta);
    vec2 b = transform({519.186f, 355.f}, theta);
    vec2 c = transform({120.814f, 355.f}, theta);

    if ((frame++ % 10) == 0) {
      if (max_tris > reset_tris) {
        max_tris = 0;
      }
      max_tris += 1;
    }

    theta += half_degree / 4;

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    triangle(a, b, c, yellow);
    // end of opaque list
    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    subdivide(a, b, c, 6);
    // end of opaque modifier list
    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    ta_wait_opaque_modifier_volume_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
