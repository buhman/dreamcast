#include <cstdint>

#include "holly/isp_tsp.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/holly.hpp"
#include "holly/core_bits.hpp"
#include "holly/core.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/video_output.hpp"
#include "holly/texture_memory_alloc2.hpp"
#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "twiddle.hpp"

#include "math/vec2.hpp"

const int _binary_model_sheik_sheik_00_data_start = 0;
const int _binary_model_sheik_sheik_00_data_size = 0;

#include "model/sheik/xc_eye01.data.h"
#include "model/sheik/sheik_00.vq.h"
#include "model/sheik/sheik_00.alpha.h"
#include "model/sheik/sheik_00.alpha.h"
#include "model/sheik/sheik_00.alpha.pal.h"
#include "model/sheik/material.h"
#include "model/sheik/model.h"

static uint32_t body_vq_offset;
static uint32_t body_alpha_offset;
static uint32_t eyes_offset;

void transfer_scene(float theta)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
					| para_control::list_type::translucent
					| obj_control::col_type::intensity_mode_1
					| obj_control::texture
					| obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::cull_if_negative;

  uint32_t tsp_instruction_word[4];
  uint32_t texture_control_word[4];
  {
    struct material_descriptor * mat = &material[body];
    tsp_instruction_word[0] = tsp_instruction_word::src_alpha_instr::one
			    | tsp_instruction_word::dst_alpha_instr::zero
			    | tsp_instruction_word::dst_select::secondary_accumulation_buffer
			    | tsp_instruction_word::fog_control::no_fog
			    | tsp_instruction_word::texture_shading_instruction::modulate
			    | tsp_instruction_word::texture_u_size::from_int(mat->pixel.width)
			    | tsp_instruction_word::texture_v_size::from_int(mat->pixel.height);

    uint32_t texture_address = texture_memory_alloc::texture.start + body_alpha_offset;
    texture_control_word[0] = texture_control_word::pixel_format::_4bpp_palette
			    | texture_control_word::scan_order::twiddled
			    | texture_control_word::texture_address(texture_address / 8);
  }
  {
    struct material_descriptor * mat = &material[body];
    tsp_instruction_word[1] = tsp_instruction_word::src_alpha_instr::dst_alpha
			    | tsp_instruction_word::dst_alpha_instr::zero
	                    | tsp_instruction_word::dst_select::secondary_accumulation_buffer
			    | tsp_instruction_word::fog_control::no_fog
			    | tsp_instruction_word::texture_shading_instruction::modulate
			    | tsp_instruction_word::texture_u_size::from_int(mat->pixel.width)
			    | tsp_instruction_word::texture_v_size::from_int(mat->pixel.height);

    uint32_t texture_address = texture_memory_alloc::texture.start + body_vq_offset;
    texture_control_word[1] = texture_control_word::vq_compressed
			    | texture_control_word::pixel_format::_565
			    | texture_control_word::scan_order::twiddled
			    | texture_control_word::texture_address(texture_address / 8);
  }
  {
    tsp_instruction_word[2] = tsp_instruction_word::src_alpha_instr::src_alpha
			    | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
	                    | tsp_instruction_word::src_select::secondary_accumulation_buffer
			    | tsp_instruction_word::fog_control::no_fog;
    texture_control_word[2] = 0;
  }
  {
    struct material_descriptor * mat = &material[eyes];
    tsp_instruction_word[3] = tsp_instruction_word::src_alpha_instr::one
			    | tsp_instruction_word::dst_alpha_instr::zero
	                    | tsp_instruction_word::src_select::primary_accumulation_buffer
			    | tsp_instruction_word::fog_control::no_fog
			    | tsp_instruction_word::texture_shading_instruction::modulate
			    | tsp_instruction_word::texture_u_size::from_int(mat->pixel.width)
			    | tsp_instruction_word::texture_v_size::from_int(mat->pixel.height);

    uint32_t texture_address = texture_memory_alloc::texture.start + eyes_offset;
    texture_control_word[3] = texture_control_word::pixel_format::_1555
			    | texture_control_word::scan_order::non_twiddled
			    | texture_control_word::texture_address(texture_address / 8);
  }


  for (int j = 0; j < sheik_model.object_count; j++) {
    struct object * object = sheik_model.object[j];

    for (int i = 0; i < object->triangle_count; i++) {
      float x[3];
      float y[3];
      float z[3];
      float u[3];
      float v[3];

      for (int k = 0; k < 3; k++) {
	int position_ix = object->triangle[i].v[k].position;
	float x0 = sheik_model.position[position_ix].x;
	float y0 = sheik_model.position[position_ix].y;
	float z0 = sheik_model.position[position_ix].z;

	float x1 = x0 * cos(theta) - z0 * sin(theta);
	float y1 = y0 - 3;
	float z1 = x0 * sin(theta) + z0 * cos(theta);

	float x2 = x1;
	float y2 = -y1;
	float z2 = z1 + 3.5;

	float x3 = x2 / z2;
	float y3 = y2 / z2;
	float z3 = z2;

	x[k] = x3 * 240 + 320;
	y[k] = y3 * 240 + 240;
	z[k] = 1 / z3;

	int texture_ix = object->triangle[i].v[k].texture;
	u[k] = sheik_model.texture[texture_ix].u;
	v[k] = 1.0 - sheik_model.texture[texture_ix].v;
      }

      for (int pass = 0; pass < 3; pass++) {
	if (object->material == eyes) {
	  pass = 3;
	}

	*reinterpret_cast<ta_global_parameter::polygon_type_1 *>(store_queue) =
	  ta_global_parameter::polygon_type_1(parameter_control_word,
					      isp_tsp_instruction_word,
					      tsp_instruction_word[pass],
					      texture_control_word[pass],
					      1.0, // face color alpha
					      1.0, // face color r
					      1.0, // face color g
					      1.0  // face color b
					      );
	sq_transfer_32byte(ta_fifo_polygon_converter);

	for (int k = 0; k < 3; k++) {
	  bool end_of_strip = k == 2;

	  *reinterpret_cast<ta_vertex_parameter::polygon_type_7 *>(store_queue) =
	    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(end_of_strip),
						x[k], y[k], z[k],
						u[k], v[k],
						1.0, // base intensity
						1.0  // offset intensity
						);
	  sq_transfer_32byte(ta_fifo_polygon_converter);
	}
      }
    }
  }

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

template <typename T>
inline void copy(T * dst, const T * src, const int32_t n) noexcept
{
  int32_t n_t = n / (sizeof (T));
  while (n_t > 0) {
    *dst++ = *src++;
    n_t--;
  }
}

void texture_init()
{
  int offset = 0;

  {
    body_alpha_offset = offset;

    uint32_t * start = (uint32_t *)&_binary_model_sheik_sheik_00_alpha_start;
    int size = (int)&_binary_model_sheik_sheik_00_alpha_size;
    copy<volatile uint32_t>(&texture_memory64[(texture_memory_alloc::texture.start + offset) / 4],
			    start,
			    size);
    offset += size;
  }

  {
    body_vq_offset = offset;

    uint32_t * start = (uint32_t *)&_binary_model_sheik_sheik_00_vq_start;
    int size = (int)&_binary_model_sheik_sheik_00_vq_size;
    copy<volatile uint32_t>(&texture_memory64[(texture_memory_alloc::texture.start + offset) / 4],
			    start,
			    size);
    offset += size;
  }

  {
    eyes_offset = offset;
    uint32_t * start = (uint32_t *)&_binary_model_sheik_xc_eye01_data_start;
    int size = (int)&_binary_model_sheik_xc_eye01_data_size;
    copy<volatile uint32_t>(&texture_memory64[(texture_memory_alloc::texture.start + offset) / 4],
			    start,
			    size);
    offset += size;
  }

  {
    uint16_t * start = (uint16_t *)&_binary_model_sheik_sheik_00_alpha_pal_start;
    int size = (int)&_binary_model_sheik_sheik_00_alpha_pal_size;
    for (int i = 0; i < size / 2; i++) {
      holly.PALETTE_RAM[i] = start[i];
    }
    holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb4444;
  }

}

void main()
{
  serial::init(4);
  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::_32x4byte
			      | ta_alloc_ctrl::om_opb::no_list
			      | ta_alloc_ctrl::o_opb::no_list
			      ;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 0,
      .opaque_modifier = 0,
      .translucent = 32 * 4,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  video_output::set_mode_vga();

  constexpr int framebuffer_width = 640;
  constexpr int framebuffer_height = 480;
  constexpr int tile_width = framebuffer_width / 32;
  constexpr int tile_height = framebuffer_height / 32;

  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array[0].start,
			 texture_memory_alloc::object_list[0].start);
  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array[1].start,
			 texture_memory_alloc::object_list[1].start);

  background_parameter2(texture_memory_alloc::background[0].start,
			0x00220033);
  background_parameter2(texture_memory_alloc::background[1].start,
			0x00220033);

  texture_init();

  const float degree = 0.017453292519943295;
  float theta = 0;
  int ta = -1;
  int core = -2;
  while (1) {
    //serial::integer<uint8_t>(ta, ' ');
    //serial::integer<uint8_t>(core, '\n');
    if (core >= 0) {
      // core = 0  ; core = 1
      // ta = 1    ; ta = 0
      core_wait_end_of_render_video();
      while (!spg_status::vsync(holly.SPG_STATUS));
      holly.FB_R_SOF1 = texture_memory_alloc::framebuffer[core].start;
      while (spg_status::vsync(holly.SPG_STATUS));
    }

    // core = -2 ; core = 1 ; core = 0
    // ta = -1   ; ta = 0   ; ta = 1
    core += 1;
    ta += 1;
    if (core > 1) core = 0;
    if (ta > 1) ta = 0;

    if (core >= 0) {
      // core = 1 ; core = 0
      // ta = 0   ; ta = 1
      ta_wait_translucent_list();

      core_start_render2(texture_memory_alloc::region_array[core].start,
			 texture_memory_alloc::isp_tsp_parameters[core].start,
			 texture_memory_alloc::background[core].start,
			 texture_memory_alloc::framebuffer[core].start,
			 framebuffer_width);
    }


    // core = -1 ; core = 1 ; core = 0
    // ta = 0    ; ta = 0   ; ta = 1
    ta_polygon_converter_init2(texture_memory_alloc::isp_tsp_parameters[ta].start,
			       texture_memory_alloc::isp_tsp_parameters[ta].end,
			       texture_memory_alloc::object_list[ta].start,
			       texture_memory_alloc::object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    transfer_scene(theta);

    theta += degree;
  }
}
