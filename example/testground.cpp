#include <stdint.h>

#include "holly/background.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/texture_memory_alloc3.hpp"
#include "holly/video_output.hpp"

#include "sh7091/serial.hpp"
#include "sh7091/store_queue.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "twiddle.hpp"
#include "memory.hpp"

#include "model/testground/testground.hpp"
#include "model/testground/maskGround.data.h"
#include "model/testground/texGrass.data.h"
#include "model/testground/texGrass2.data.h"
#include "model/testground/texRock.data.h"

static float theta = 0;
static int frame = 0;
const float degree = 0.017453292519943295 / 2;
const float max_yz_theta = degree * 100;

const uint32_t mask_texture_address   = texture_memory_alloc.texture.start + 0x00000;
const uint32_t rock_texture_address   = texture_memory_alloc.texture.start + 0x20000;
const uint32_t grass_texture_address  = texture_memory_alloc.texture.start + 0x28000;
const uint32_t grass2_texture_address = texture_memory_alloc.texture.start + 0x30000;

static inline vec3 transform_vertex(vec3 vec)
{
  float x9 = vec.x - 2;
  float y9 = vec.y;
  float z9 = vec.z - 2;

  float yz_rotate_theta = theta >= max_yz_theta ? max_yz_theta : theta;

  float x0 = x9 * cos(theta) - z9 * sin(theta);
  float y0 = y9;
  float z0 = x9 * sin(theta) + z9 * cos(theta);

  float x1 = x0;
  float y1 = y0 * cos(yz_rotate_theta) - z0 * sin(yz_rotate_theta);
  float z1 = y0 * sin(yz_rotate_theta) + z0 * cos(yz_rotate_theta);

  float x2 = x1;
  float y2 = y1;
  float z2 = z1 + 12;

  z2 *= 0.6;
  float x3 = (x2 + 0) / z2;
  float y3 = (y2 - 5) / z2;
  float z3 = 1.0 / z2;

  float x = x3 * 240 + 320;
  float y = y3 * 240 + 320;
  float z = z3;

  return {x, y, z};
}

const uint32_t base_color = 0xffc0c000;

static inline void transfer_triangle(int index, const vec2 * texcoord, float uv_scale)
{
  vec3 v1 = transform_vertex(position[indices[index + 0]]);
  vec2 uv1 = texcoord[indices[index + 0]];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.u * uv_scale, uv1.v * uv_scale,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[indices[index + 1]]);
  vec2 uv2 = texcoord[indices[index + 1]];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.u * uv_scale, uv2.v * uv_scale,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[indices[index + 2]]);
  vec2 uv3 = texcoord[indices[index + 2]];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.u * uv_scale, uv3.v * uv_scale,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

const int triangle_count = ((sizeof (indices)) / (sizeof (indices[0]))) / 3;

static inline void transfer_triangles(const uint32_t tsp_instruction_word, const uint32_t texture_control_word, const vec2 * texcoord, float uv_scale)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  int index = 0;
  for (int i = 0; i < triangle_count; i++) {
    transfer_triangle(index, texcoord, uv_scale);
    index += 3;
  }
}

void transfer_scene()
{
  // mask: swizzle palette 1
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(512)
                                        | tsp_instruction_word::texture_v_size::from_int(256)
      //                                        | tsp_instruction_word::use_alpha
      ;

    const uint32_t texture_address = mask_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8)
                                        | texture_control_word::palette_selector8(1);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_0, 1.0f);
  }

  // rock texture
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::dst_alpha
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(128)
                                        | tsp_instruction_word::texture_v_size::from_int(128)
      //| tsp_instruction_word::use_alpha
      ;

    const uint32_t texture_address = rock_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_1, 5.0f);
  }

  // mask: swizzle palette 2 ; secondary accumulation buffer
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(512)
                                        | tsp_instruction_word::texture_v_size::from_int(256)
      //| tsp_instruction_word::use_alpha
                                        | tsp_instruction_word::src_select::primary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::secondary_accumulation_buffer;

    const uint32_t texture_address = mask_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8)
                                        | texture_control_word::palette_selector8(2);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_0, 1.0f);
  }

  // grass texture ; secondary accumulation buffer
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::dst_alpha
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(128)
                                        | tsp_instruction_word::texture_v_size::from_int(128)
      //| tsp_instruction_word::use_alpha
                                        | tsp_instruction_word::src_select::primary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::secondary_accumulation_buffer;

    const uint32_t texture_address = grass_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_1, 5.0f);
  }

  // flush secondary accumulation buffer to primary
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
                                        | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::src_select::secondary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::primary_accumulation_buffer;

    const uint32_t texture_control_word = 0;

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_1, 5.0f);
  }

  // mask: swizzle palette 3 ; secondary accumulation buffer
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(512)
                                        | tsp_instruction_word::texture_v_size::from_int(256)
      //| tsp_instruction_word::use_alpha
                                        | tsp_instruction_word::src_select::primary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::secondary_accumulation_buffer;

    const uint32_t texture_address = mask_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8)
                                        | texture_control_word::palette_selector8(3);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_0, 1.0f);
  }

  // grass2 texture ; secondary accumulation buffer
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::dst_alpha
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(128)
                                        | tsp_instruction_word::texture_v_size::from_int(128)
      //| tsp_instruction_word::use_alpha
                                        | tsp_instruction_word::src_select::primary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::secondary_accumulation_buffer;

    const uint32_t texture_address = grass2_texture_address;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                        | texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address(texture_address / 8);

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_1, 1.0f);
  }

  // flush secondary accumulation buffer to primary
  if (1) {
    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
                                        | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::src_select::secondary_accumulation_buffer
                                        | tsp_instruction_word::dst_select::primary_accumulation_buffer;

    const uint32_t texture_control_word = 0;

    transfer_triangles(tsp_instruction_word, texture_control_word, texcoord_1, 1.0f);
  }

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit
  system.LMMODE1 = 0; // 64-bit

  // mask
  {
    uint8_t * start = (uint8_t *)&_binary_model_testground_maskGround_data_start;
    uint8_t twiddle_temp[512 * 256] __attribute__((aligned(4)));
    twiddle::texture<uint8_t>(twiddle_temp, start, 512, 256);
    //memory::copy<volatile uint32_t>(&ta_fifo_texture_memory[mask_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 512 * 256);
    memory::copy<volatile uint32_t>(&texture_memory64[mask_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 512 * 256);
  }

  // rock
  {
    uint16_t * start = (uint16_t *)&_binary_model_testground_texRock_data_start;
    uint16_t twiddle_temp[128 * 128] __attribute__((aligned(4)));
    twiddle::texture<uint16_t>(twiddle_temp, start, 128, 128);
    //memory::copy<volatile uint32_t>(&ta_fifo_texture_memory[rock_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
    memory::copy<volatile uint32_t>(&texture_memory64[rock_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
  }

  // grass
  {
    uint16_t * start = (uint16_t *)&_binary_model_testground_texGrass_data_start;
    uint16_t twiddle_temp[128 * 128] __attribute__((aligned(4)));
    twiddle::texture<uint16_t>(twiddle_temp, start, 128, 128);
    //memory::copy<volatile uint32_t>(&ta_fifo_texture_memory[grass_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
    memory::copy<volatile uint32_t>(&texture_memory64[grass_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
  }

  // grass2
  {
    uint16_t * start = (uint16_t *)&_binary_model_testground_texGrass2_data_start;
    uint16_t twiddle_temp[128 * 128] __attribute__((aligned(4)));
    twiddle::texture<uint16_t>(twiddle_temp, start, 128, 128);
    //memory::copy<volatile uint32_t>(&ta_fifo_texture_memory[grass2_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
    memory::copy<volatile uint32_t>(&texture_memory64[grass2_texture_address / 4], reinterpret_cast<uint32_t *>(&twiddle_temp[0]), 128 * 128);
  }
}

static inline uint16_t argb1555(int a, int r, int g, int b)
{
  return ((a & 1) << 15) | ((r & 31) << 10) | ((g & 31) << 5) | ((b & 31) << 0);
}

void transfer_palette_ram()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;
  // debug palette
  holly.PALETTE_RAM[  0 + 0] = argb1555(1, 31,  0,  0);
  holly.PALETTE_RAM[  0 + 1] = argb1555(1,  0, 31,  0);
  holly.PALETTE_RAM[  0 + 2] = argb1555(1,  0,  0, 31);

  // "swizzle" palettes
  holly.PALETTE_RAM[256 + 0] = argb1555(1,  0,  0,  0);
  holly.PALETTE_RAM[256 + 1] = argb1555(0,  0,  0,  0);
  holly.PALETTE_RAM[256 + 2] = argb1555(0,  0,  0,  0);

  holly.PALETTE_RAM[512 + 0] = argb1555(0,  0,  0,  0);
  holly.PALETTE_RAM[512 + 1] = argb1555(1,  0,  0,  0);
  holly.PALETTE_RAM[512 + 2] = argb1555(0,  0,  0,  0);

  holly.PALETTE_RAM[768 + 0] = argb1555(0,  0,  0,  0);
  holly.PALETTE_RAM[768 + 1] = argb1555(0,  0,  0,  0);
  holly.PALETTE_RAM[768 + 2] = argb1555(1,  0,  0,  0);
}

void main()
{
  for (int i = 0; i < 8 * 1024 * 1024 / 4; i++) {
    ta_fifo_texture_memory[i] = 0;
  }

  transfer_textures();
  transfer_palette_ram();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::_16x4byte
			      | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::no_list;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 0,
      .opaque_modifier = 0,
      .translucent = 16 * 4,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  video_output::set_mode_vga();

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc.region_array[0].start,
			 texture_memory_alloc.object_list[1].start);

  //texture_memory_alloc.background[1].start;
  background_parameter2(0x7fffe0 - 128,
			0xff220033);

  frame = 0;

  while (1) {
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[0].start,
			       texture_memory_alloc.isp_tsp_parameters[0].end,
			       texture_memory_alloc.object_list[1].start,
			       texture_memory_alloc.object_list[1].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    transfer_scene();
    ta_wait_translucent_list();

    holly.SOFTRESET = softreset::pipeline_soft_reset;
    holly.SOFTRESET = 0;
    core_start_render2(texture_memory_alloc.region_array[0].start,
                       texture_memory_alloc.isp_tsp_parameters[0].start,
                       0x7fffe0 - 128,//texture_memory_alloc.background[1].start,
                       texture_memory_alloc.framebuffer[frame & 1].start,
                       framebuffer_width);

    //core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    while (spg_status::vsync(holly.SPG_STATUS));

    frame += 1;

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[frame & 1].start;
    while (spg_status::vsync(holly.SPG_STATUS));

    theta += degree;
  }
  serial::string("return\nreturn\nreturn\n");
}
