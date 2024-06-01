#include <cstdint>

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

#include "sh7091/store_queue.hpp"

#include "math/vec3.hpp"

constexpr float isqrt2 = 0.7071067811865475;

using vec3 = vec<3, float>;

void triangle(vec3 a, vec3 b, vec3 c, const uint32_t base_color)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
					tsp_instruction_word,
                                        0, // texture_control_word
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
					a.x, a.y, a.z,
					base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
					b.x, b.y, b.z,
					base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
					c.x, c.y, c.z,
					base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void tetrahedron(vec3 a, vec3 b, vec3 c, vec3 d)
{
  /*
    vec3 a = { 1, 0,-isqrt2};
    vec3 b = {-1, 0,-isqrt2};
    vec3 c = { 0, 1, isqrt2};
    vec3 d = { 0,-1, isqrt2};

      c - d
     / \ /
    b - a
  */

  triangle(a, b, c, 0xff'00ffff); // c
  triangle(a, c, d, 0xff'ffff00); // y
  triangle(d, a, b, 0xff'ff00ff); // m
  triangle(d, b, c, 0xff'00ff00); // g
}

vec3 midpoint(vec3 a, vec3 b)
{
  return {(a.x + b.x) / 2.f,
	  (a.y + b.y) / 2.f,
	  (a.z + b.z) / 2.f};
}

void subdivide(vec3 a, vec3 b, vec3 c, vec3 d, int depth)
{
  vec3 ab = midpoint(a, b);
  vec3 ac = midpoint(a, c);
  vec3 ad = midpoint(a, d);
  vec3 bc = midpoint(b, c);
  vec3 bd = midpoint(b, d);
  vec3 cd = midpoint(c, d);

  if (depth == 0) {
    tetrahedron(ad,  a, ab, ac);
    tetrahedron(bd, ab,  b, bc);
    tetrahedron(cd, ac, bc,  c);
    tetrahedron( d, ad, bd, cd);
  } else {
    subdivide(ad,  a, ab, ac, depth - 1);
    subdivide(bd, ab,  b, bc, depth - 1);
    subdivide(cd, ac, bc,  c, depth - 1);
    subdivide( d, ad, bd, cd, depth - 1);
  }
}

float theta;

vec3 transform(vec3 v)
{
  float x1 = v.x * __builtin_cosf(theta) - v.z * __builtin_sinf(theta);
  float y1 = v.y;
  float z1 = v.x * __builtin_sinf(theta) + v.z * __builtin_cosf(theta);

  float x = x1;
  float y = y1 * __builtin_cosf(-theta * 0.3) - z1 * __builtin_sinf(-theta * 0.3);
  float z = y1 * __builtin_sinf(-theta * 0.3) + z1 * __builtin_cosf(-theta * 0.3);

  return {
    x * 200.f + 320.f,
    y * 200.f + 240.f,
    1.f / (z + 10.f),
  };
}

void main()
{
  video_output::set_mode_vga();
  theta = 0.f;

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

  uint32_t frame_ix = 0;

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    vec3 a = { 1, 0,-isqrt2};
    vec3 b = {-1, 0,-isqrt2};
    vec3 c = { 0, 1, isqrt2};
    vec3 d = { 0,-1, isqrt2};

    subdivide(transform(a),
	      transform(b),
	      transform(c),
	      transform(d),
	      4);

    theta += 0.01f;

    // end of opaque list
    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
