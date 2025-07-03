#include <cstdint>

#include "align.hpp"
#include "holly/video_output.hpp"

#include "holly/texture_memory_alloc9.hpp"
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
#include "memorymap.hpp"

#include "math/float_types.hpp"

#include "sh7091/serial.hpp"

#include "holly/object_list_data.hpp"

constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::_8x4byte
                            | ta_alloc_ctrl::tm_opb::no_list
                            | ta_alloc_ctrl::t_opb::no_list
                            | ta_alloc_ctrl::om_opb::_8x4byte
                            | ta_alloc_ctrl::o_opb::no_list;

const struct opb_size opb_size = { .opaque = 0
                                 , .opaque_modifier = 8 * 4
                                 , .translucent = 0
                                 , .translucent_modifier = 0
                                 , .punch_through = 8 * 4
                                 };

constexpr int tile_width = 32 / 32;
constexpr int tile_height = 32 / 32;

void init_texture_memory()
{
  region_array_multipass(tile_width,
			 tile_height,
			 &opb_size,
			 1,
			 texture_memory_alloc.region_array.start,
			 texture_memory_alloc.object_list.start);

  background_parameter(0xff222200);
}

constexpr float half_degree = 0.01745329f / 2.f;

struct triangle_parameter_vertex {
  float x;
  float y;
  float z;
  uint32_t color1;
  uint32_t color2;
};

struct triangle_parameter {
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word_0;
  uint32_t texture_control_word_0;
  uint32_t tsp_instruction_word_1;
  uint32_t texture_control_word_1;
  triangle_parameter_vertex a;
  triangle_parameter_vertex b;
  triangle_parameter_vertex c;
};

struct modifier_volume_parameter_vertex {
  float x;
  float y;
  float z;
};

struct modifier_volume_parameter {
  uint32_t isp_tsp_instruction_word;
  uint32_t pad1;
  uint32_t pad2;
  modifier_volume_parameter_vertex a;
  modifier_volume_parameter_vertex b;
  modifier_volume_parameter_vertex c;
};

template <int N>
struct object_pointer_block {
  uint32_t pointer[N];
};
static_assert((sizeof (object_pointer_block<8>)) == 32);

template <int N>
void transfer_object_list(volatile uint8_t * mem)
{
  auto blocks = reinterpret_cast<volatile object_pointer_block<N> *>(mem);

  { // opaque modifier
    int start = (sizeof (triangle_parameter));

    auto& block = blocks[0];
    block.pointer[0] = object_list_data::pointer_type::triangle_array
                     | object_list_data::triangle_array::number_of_triangles(0)
                     | object_list_data::triangle_array::skip(0)
                     | object_list_data::triangle_array::start(start / 4);

    block.pointer[1] = object_list_data::pointer_type::object_pointer_block_link
                     | object_list_data::object_pointer_block_link::end_of_list;
  }

  { // punch through
    int start = 0;

    auto& block = blocks[1];
    block.pointer[0] = object_list_data::pointer_type::triangle_array
                     | object_list_data::triangle_array::number_of_triangles(0)
                     | object_list_data::triangle_array::shadow
                     | object_list_data::triangle_array::skip(1)
                     | object_list_data::triangle_array::start(start / 4);

    block.pointer[1] = object_list_data::pointer_type::object_pointer_block_link
                     | object_list_data::object_pointer_block_link::end_of_list;
  }
}

using vec2i = vec<2, int>;

struct triangle {
  vec3 a;
  vec3 b;
  vec3 c;
};

static inline int transfer_isp_tsp_parameters(volatile uint8_t * mem,
                                              const vec3& ap, const vec2i& ac,
                                              const vec3& bp, const vec2i& bc,
                                              const vec3& cp, const vec2i& cc)
{
  auto params = reinterpret_cast<volatile triangle_parameter *>(mem);
  params->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                   | isp_tsp_instruction_word::culling_mode::no_culling
                                   | isp_tsp_instruction_word::gouraud_shading;

  params->tsp_instruction_word_0 = tsp_instruction_word::src_alpha_instr::one
                                 | tsp_instruction_word::dst_alpha_instr::zero
                                 | tsp_instruction_word::fog_control::no_fog;

  params->texture_control_word_0 = 0;

  params->tsp_instruction_word_1 = tsp_instruction_word::src_alpha_instr::one
                                 | tsp_instruction_word::dst_alpha_instr::one
                                 | tsp_instruction_word::fog_control::no_fog;

  params->texture_control_word_1 = 0;

  params->a.x = ap.x;
  params->a.y = ap.y;
  params->a.z = ap.z;
  params->a.color1 = ac.x;
  params->a.color2 = ac.y;

  params->b.x = bp.x;
  params->b.y = bp.y;
  params->b.z = bp.z;
  params->b.color1 = bc.x;
  params->b.color2 = bc.y;

  params->c.x = cp.x;
  params->c.y = cp.y;
  params->c.z = cp.z;
  params->c.color1 = cc.x;
  params->c.color2 = cc.y;

  return (sizeof (triangle_parameter));
}

static inline int transfer_modifier_volume_isp_tsp_parameters(volatile uint8_t * mem,
                                                              const vec3& ap,
                                                              const vec3& bp,
                                                              const vec3& cp)
{
  auto params = reinterpret_cast<volatile modifier_volume_parameter *>(mem);
  params->isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                   | isp_tsp_instruction_word::culling_mode::no_culling;

  params->pad1 = 0;
  params->pad2 = 0;

  params->a.x = ap.x;
  params->a.y = ap.y;
  params->a.z = ap.z;

  params->b.x = bp.x;
  params->b.y = bp.y;
  params->b.z = bp.z;

  params->c.x = cp.x;
  params->c.y = cp.y;
  params->c.z = cp.z;

  return (sizeof (modifier_volume_parameter));
}

void main()
{
  video_output::set_mode_vga();

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory();
  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

  uint32_t frame_ix = 0;

  constexpr triangle q = {
    {18.2192f,     1.0f, 0.01f},
    {27.8808f, 25.4219f, 0.01f},
    {    1.9f, 21.5781f, 0.01f},
  };

  constexpr triangle mv = {
    {  0.0f, -50.0f, 0.1f},
    {100.0f,   0.0f, 0.1f},
    {  0.0f,  50.0f, 0.1f},
  };

  auto object_list = (volatile uint8_t *)(&texture_memory32[texture_memory_alloc.object_list.start / 4]);
  auto isp_tsp_parameters = (volatile uint8_t *)(&texture_memory32[texture_memory_alloc.isp_tsp_parameters.start / 4]);

  while (true) {
    vec3 ap = q.a;
    vec3 bp = q.b;
    vec3 cp = q.c;

    vec2i ac = {0x0000ff, 0xff0000};
    vec2i bc = {0x00ff00, 0x0000ff};
    vec2i cc = {0xff0000, 0x00ff00};

    transfer_object_list<8>(object_list);

    int offset = 0;
    offset += transfer_isp_tsp_parameters(&isp_tsp_parameters[offset],
                                          ap, ac,
                                          bp, bc,
                                          cp, cc);

    offset += transfer_modifier_volume_isp_tsp_parameters(&isp_tsp_parameters[offset],
                                                          mv.a,
                                                          mv.b,
                                                          mv.c);

    int framebuffer_width = 640;
    bool dither = true;
    core_start_render2(texture_memory_alloc.region_array.start,
                       texture_memory_alloc.isp_tsp_parameters.start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[frame_ix].start,
                       framebuffer_width,
                       dither);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[frame_ix].start;
    while (spg_status::vsync(holly.SPG_STATUS));

    //frame_ix = (frame_ix + 1) & 1;

    return;
  }
}
