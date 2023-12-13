#include <cstdint>

#include "align.hpp"

#include "vga.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"

struct vertex {
  float x;
  float y;
  float z;
};

// screen space coordinates
const struct vertex quad_verticies[4] = {
  { 200.f,  360.f, 0.1f },
  { 200.f,  120.f, 0.1f },
  { 440.f,  120.f, 0.1f },
  { 440.f,  360.f, 0.1f },
};

uint32_t transform(uint32_t * ta_parameter_buf)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);

  constexpr uint32_t base_color = 0xffff0000;
  parameter.append<global_sprite>() = global_sprite(base_color);
  parameter.append<vertex_sprite_type_0>() =
    vertex_sprite_type_0(quad_verticies[0].x,
			 quad_verticies[0].y,
			 quad_verticies[0].z,
			 quad_verticies[1].x,
			 quad_verticies[1].y,
			 quad_verticies[1].z,
			 quad_verticies[2].x,
			 quad_verticies[2].y,
			 quad_verticies[2].z,
			 quad_verticies[3].x,
			 quad_verticies[3].y);
  // curiously, there is no quad_veritices[3].z in vertex_sprite_type_0

  parameter.append<global_end_of_list>() = global_end_of_list();

  return parameter.offset;
}

uint32_t _ta_parameter_buf[((32 + 64 + 32) + 32) / 4];

void main()
{
  vga();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  core_init_texture_memory();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  while (true) {
    v_sync_out();
    v_sync_in();

    ta_polygon_converter_init();
    uint32_t ta_parameter_size = transform(ta_parameter_buf);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    constexpr int frame_ix = 0;
    constexpr int num_frames = 0;
    core_start_render(frame_ix, num_frames);
  }
}
