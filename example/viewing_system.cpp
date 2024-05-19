#include <cstdint>

#include "align.hpp"

#include "holly/video_output.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/isp_tsp.hpp"
#include "memorymap.hpp"

#include "geometry/geometry.hpp"
#include "geometry/suzanne2.hpp"
#include "geometry/plane2.hpp"

#include "viewing_system/view_space.hpp"
#include "viewing_system/screen_space.hpp"

uint32_t _ta_parameter_buf[((32 * 8192) + 32) / 4];

struct viewer {
  vec3 position;
  vec3 orientation;
  float azimuth;
  float colatitude;
};

void ta_upload(ta_parameter_writer& parameter,
               const position__color * vertices,
               const face_vtn * faces,
               const uint32_t num_faces,
	       const mat4x4 world_transform,
               const mat4x4 screen_transform
               )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::floating_color
					| obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  for (uint32_t face_ix = 0; face_ix < num_faces; face_ix++) {
    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );

    auto& face = faces[face_ix];
    constexpr uint32_t strip_length = 3;
    mat4x4 transform = screen_transform * world_transform;
    for (uint32_t i = 0; i < strip_length; i++) {
      const uint32_t vertex_ix = face[i].vertex;
      auto& position = vertices[vertex_ix].position;
      auto& color = vertices[vertex_ix].color;
      vec4 vertex = { position.x,
                      position.y,
                      position.z,
                      1.0f       };

      // in three-dimensional screen space
      vec4 v = transform * vertex;

      float x = v.x / v.w;
      float y = -v.y / v.w;
      float z = v.w / v.z;

      x = x * 240.f + 320.f;
      y = y * 240.f + 240.f;

      // perspective divide
      bool end_of_strip = i == strip_length - 1;
      parameter.append<ta_vertex_parameter::polygon_type_1>() =
	ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					    x,
					    y,
					    z,
					    1.0f, // alpha
					    color.r, // red
					    color.g, // green
					    color.b  // blue
					    );
    }
  }
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff220000);
}

void main()
{
  video_output::set_mode_vga();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::no_list
			      | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
				       , .opaque_modifier = 0
				       , .translucent = 0
				       , .translucent_modifier = 0
				       , .punch_through = 0
				       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  float delta = 0;

  uint32_t frame_ix = 0;

  while (true) {

    viewer viewer {
      .position    = {0.f, -1.f, -2.f},
      .orientation = {0.f, -1.f,  0.f}, // approximate "up" orientation
      .azimuth     = 0,
      .colatitude  = pi / 4.f * sin(delta) * 0.9f,
    };

    const vec3 plane_normal = view_space::viewing_direction(viewer.azimuth, viewer.colatitude);
    const vec3 up_vector = view_space::project_vector_to_plane(plane_normal, viewer.orientation);

    const mat4x4 view_space_transform = view_space::transformation_matrix(viewer.position, plane_normal, up_vector);

    const mat4x4 perspective_transform = screen_space::transformation_matrix(1.f,   // the z-coordinate of the view window
									     100.f, // the z-coordinate of the far clip plane
									     1.f    // the dimension of the square view window
									     );

    const mat4x4 screen_transform = perspective_transform * view_space_transform;


    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);
    auto parameter = ta_parameter_writer(ta_parameter_buf);


    {
      constexpr mat4x4 world_transform = { 1.f, 0.f, 0.f, 0.f,
					   0.f, 1.f, 0.f, 0.f,
					   0.f, 0.f, 1.f, 3.f,
					   0.f, 0.f, 0.f, 1.f };
      ta_upload(parameter,
		suzanne::vertices,
		suzanne::faces,
		suzanne::num_faces,
		world_transform,
		screen_transform
		);
    }

    {
      constexpr mat4x4 world_transform = { 0.1f, 0.f,  0.f,  0.f,
					   0.f,  0.1f, 0.f,  1.2f,
					   0.f,  0.f,  0.1f, 3.f,
					   0.f,  0.f,  0.f,  1.f };
      ta_upload(parameter,
		plane::vertices,
		plane::faces,
		plane::num_faces,
		world_transform,
		screen_transform
		);
    }

    // end of opaque list
    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
    delta += pi * 2 / 360;
  }
}
