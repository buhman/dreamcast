#include <cstdint>

#include "align.hpp"

#include "vga.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "memorymap.hpp"

#include "serial.hpp"

#include "macaw.hpp"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

vertex cube_faces[][4] = {
  { // front
    { -1.0f, -1.0f,  1.0f, 0.f, 0.f},
    { -1.0f,  1.0f,  1.0f, 0.f, 1.f},
    {  1.0f, -1.0f,  1.0f, 1.f, 0.f},
    {  1.0f,  1.0f,  1.0f, 1.f, 1.f},
  },
  { // back
    { -1.0f, -1.0f, -1.0f, 1.f, 0.f},
    { -1.0f,  1.0f, -1.0f, 1.f, 1.f},
    {  1.0f, -1.0f, -1.0f, 0.f, 0.f},
    {  1.0f,  1.0f, -1.0f, 0.f, 1.f},
  },
  { // right side
    {  1.0f, -1.0f,  1.0f, 0.f, 0.f},
    {  1.0f,  1.0f,  1.0f, 0.f, 1.f},
    {  1.0f, -1.0f, -1.0f, 1.f, 0.f},
    {  1.0f,  1.0f, -1.0f, 1.f, 1.f},
  },
  { // left side
    {  -1.0f, -1.0f,  1.0f, 1.f, 0.f},
    {  -1.0f,  1.0f,  1.0f, 1.f, 1.f},
    {  -1.0f, -1.0f, -1.0f, 0.f, 0.f},
    {  -1.0f,  1.0f, -1.0f, 0.f, 1.f},
  },
};
constexpr uint32_t num_faces = (sizeof (cube_faces)) / (sizeof (cube_faces[0]));

constexpr uint32_t color = 0xffff00ff;

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

void transform(ta_parameter_writer& parameter,
		   const vertex * strip_vertices,
		   const uint32_t strip_length)
{
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));
  parameter.append<global_polygon_type_0>() = global_polygon_type_0(texture_address);

  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float t;

    t  = y * __builtin_cosf(theta) - z * __builtin_sinf(theta);
    z  = y * __builtin_sinf(theta) + z * __builtin_cosf(theta);
    y  = t;

    float theta2 = 3.14 * __builtin_sinf(theta / 2);

    t  = x * __builtin_cosf(theta2) - z * __builtin_sinf(theta2);
    z  = x * __builtin_sinf(theta2) + z * __builtin_cosf(theta2);
    x  = t;

    z += 3;

    // perspective
    x = x / z;
    y = y / z;

    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;


    z = 1 / z;

    parameter.append<vertex_polygon_type_3>() =
      vertex_polygon_type_3(x, y, z,
			    strip_vertices[i].u,
			    strip_vertices[i].v,
			    color,
			    end_of_strip);
  }
}

void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background);

  region_array2(mem->region_array,
	        (offsetof (struct texture_memory_alloc, object_list)),
		640 / 32, // width
		480 / 32, // height
		opb_size
		);
}

void copy_macaw_texture()
{
  auto src = reinterpret_cast<const uint8_t *>(&_binary_macaw_data_start);
  auto size  = reinterpret_cast<const uint32_t>(&_binary_macaw_data_size);
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory64);
  for (uint32_t px = 0; px < size / 3; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    mem->texture[px] = rgb565;
  }
}

uint32_t _ta_parameter_buf[((32 * (5 * 6 + 1)) + 32) / 4];

void main()
{
  vga();
  copy_macaw_texture();

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

  constexpr uint32_t tiles = (640 / 32) * (320 / 32);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;
  constexpr uint32_t num_frames = 1;

  while (1) {
    ta_polygon_converter_init(opb_size.total() * tiles, ta_alloc);
    auto parameter = ta_parameter_writer(ta_parameter_buf);
    for (uint32_t i = 0; i < num_faces; i++) {
      transform(parameter, cube_faces[i], 4);
    }
    parameter.append<global_end_of_list>() = global_end_of_list();
    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();
    core_start_render(frame_ix, num_frames);

    v_sync_in();
    core_wait_end_of_render_video(frame_ix, num_frames);
    theta += half_degree;
    frame_ix += 1;
  }
}