#include <cstdint>

#include "align.hpp"
#include "vga.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"

#include "macaw.hpp"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t color;
};

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv coordinates       ]  [color   ]
  { -0.5f,   0.5f,  0.f, 0.f        , 127.f/128.f, 0x00000000}, // the first two base colors in a
  { -0.5f,  -0.5f,  0.f, 0.f        , 0.f        , 0x00000000}, // non-Gouraud triangle strip are ignored
  {  0.5f,   0.5f,  0.f, 127.f/128.f, 127.f/128.f, 0xffff00ff},
  {  0.5f,  -0.5f,  0.f, 127.f/128.f, 0.f        , 0xffffff00},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

uint32_t transform(uint32_t * ta_parameter_buf,
		   const vertex * strip_vertices,
		   const uint32_t strip_length)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));
  parameter.append<global_polygon_type_0>() = global_polygon_type_0(texture_address);

  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float x1;

    x1 = x * __builtin_cosf(theta) - z * __builtin_sinf(theta);
    z  = x * __builtin_sinf(theta) + z * __builtin_cosf(theta);
    x  = x1;
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;
    z = 1.f / (z + 10.f);

    parameter.append<vertex_polygon_type_3>() =
      vertex_polygon_type_3(x, y, z,
			    strip_vertices[i].u,
			    strip_vertices[i].v,
			    strip_vertices[i].color,
			    end_of_strip);
  }

  parameter.append<global_end_of_list>() = global_end_of_list();

  theta += half_degree;

  return parameter.offset;
}

uint32_t _ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4];

void main()
{
  vga();

  auto src = reinterpret_cast<const uint8_t *>(&_binary_macaw_data_start);
  auto size  = reinterpret_cast<const uint32_t>(&_binary_macaw_data_size);
  auto mem = reinterpret_cast<texture_memory_alloc *>(0xa400'0000);
  for (uint32_t px = 0; px < size / 3; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    mem->texture[px] = rgb565;
  }

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
    uint32_t ta_parameter_size = transform(ta_parameter_buf, strip_vertices, strip_length);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    constexpr int frame_ix = 0;
    constexpr int num_frames = 0;
    core_start_render(frame_ix, num_frames);
  }
}
