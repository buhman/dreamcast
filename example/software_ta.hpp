#include <cstdint>

#include "holly/object_list_data.hpp"
#include "holly/isp_tsp.hpp"

namespace software_ta {

constexpr uint32_t tile_size = 32;

constexpr uint32_t framebuffer_width = 640;
constexpr uint32_t framebuffer_height = 480;

constexpr uint32_t tile_width = framebuffer_width / tile_size;
constexpr uint32_t tile_height = framebuffer_height / tile_size;

struct point {
  float x;
  float y;
};

struct quad {
  point a;
  point b;
  point c;
  point d;
};

struct bounding_box {
  point min;
  point max;

  bounding_box(const quad q)
  {
    min.x = (q.a.x < q.b.x) ? q.a.x : q.b.x;
    min.x = (min.x < q.c.x) ? min.x : q.c.x;
    min.x = (min.x < q.d.x) ? min.x : q.d.x;

    min.y = (q.a.y < q.b.y) ? q.a.y : q.b.y;
    min.y = (min.y < q.c.y) ? min.y : q.c.y;
    min.y = (min.y < q.d.y) ? min.y : q.d.y;

    max.x = (q.a.x > q.b.x) ? q.a.x : q.b.x;
    max.x = (max.x > q.c.x) ? max.x : q.c.x;
    max.x = (max.x > q.d.x) ? max.x : q.d.x;

    max.y = (q.a.y > q.b.y) ? q.a.y : q.b.y;
    max.y = (max.y > q.c.y) ? max.y : q.c.y;
    max.y = (max.y > q.d.y) ? max.y : q.d.y;
  }
};

template <int32_t max>
constexpr uint32_t clamp_int(const float x)
{
  const int32_t n = static_cast<int32_t>(x);
  return (n < 0 ? 0 : (n >= max ? (max - 1) : x));
}

constexpr uint32_t tile_start_x(bounding_box bb)
{
  return clamp_int<framebuffer_width>(__builtin_floorf(bb.min.x / tile_size));
}

constexpr uint32_t tile_start_y(bounding_box bb)
{
  return clamp_int<framebuffer_height>(__builtin_floorf(bb.min.y / tile_size));
}

constexpr uint32_t tile_end_x(bounding_box bb)
{
  return clamp_int<framebuffer_height>(__builtin_ceilf(bb.max.x / tile_size));
}

constexpr uint32_t tile_end_y(bounding_box bb)
{
  return clamp_int<framebuffer_height>(__builtin_ceilf(bb.max.y / tile_size));
}

template <int N>
struct object_pointer_block {
  uint32_t pointer[N];
};
static_assert((sizeof (object_pointer_block<8>)) == 32);

template <int N>
void object_pointer_blocks(volatile uint32_t * mem, const quad& quad)
{
  auto block = reinterpret_cast<volatile object_pointer_block<N> *>(mem);
  uint8_t tile_indices[tile_height][tile_width];
  for (uint32_t i = 0; i < tile_width * tile_height; i++) {
    reinterpret_cast<uint8_t *>(tile_indices)[i] = 0;
  }

  bounding_box bb(quad);

  for (uint32_t y = tile_start_y(bb); y < tile_end_y(bb); y++) {
    for (uint32_t x = tile_start_x(bb); x < tile_end_x(bb); x++) {
      uint8_t& ix = tile_indices[y][x];
      auto& opb = block[y * tile_width + x];
      opb.pointer[ix] = object_list_data::pointer_type::quad_array
                      | object_list_data::quad_array::number_of_quads(0)
                      | object_list_data::quad_array::skip(1)
                      | object_list_data::quad_array::start(0);
      ix++;
    }
  }

  for (uint32_t y = 0; y < tile_height; y++) {
    for (uint32_t x = 0; x < tile_width; x++) {
      uint8_t& ix = tile_indices[y][x];
      auto& opb = block[y * tile_width + x];
      opb.pointer[ix] = object_list_data::pointer_type::object_pointer_block_link
                      | object_list_data::object_pointer_block_link::end_of_list;
      ix++;
    }
  }
}

struct __untextured_quad_vertex {
  float x;
  float y;
  float z;
  uint32_t color;
};

struct __untextured_quad {
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  __untextured_quad_vertex a;
  __untextured_quad_vertex b;
  __untextured_quad_vertex c;
  __untextured_quad_vertex d;
};

void isp_tsp_parameters(volatile uint32_t * mem, const quad& quad)
{
  auto params = reinterpret_cast<volatile __untextured_quad *>(mem);
  params->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                   | isp_tsp_instruction_word::culling_mode::no_culling;

  params->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                               | tsp_instruction_word::dst_alpha_instr::zero
                               | tsp_instruction_word::fog_control::no_fog;

  params->texture_control_word = 0;

  params->a.x = quad.a.x;
  params->a.y = quad.a.y;
  params->a.z = 0.1f;
  params->b.color = 0; // invalid

  params->b.x = quad.b.x;
  params->b.y = quad.b.y;
  params->b.z = 0.1f;
  params->b.color = 0; // invalid

  params->c.x = quad.c.x;
  params->c.y = quad.c.y;
  params->c.z = 0.1f;
  params->c.color = 0xffff0000;

  params->d.x = quad.d.x;
  params->d.y = quad.d.y;
  params->d.z = 0.f; // invalid
  params->b.color = 0; // invalid
}

}

/*
int main()
{
  using namespace software_ta;

  quad q = {
    {200.f, 360.f},
    {200.f, 120.f},
    {440.f, 120.f},
    {440.f, 360.f},
  };

  uint32_t opb_mem[300 * 8];

  object_pointer_blocks<8>(opb_mem, q);
  for (int i = 0; i < 300 * 8; i++) {
    if ((i & 7) == 0)
      std::cout << '\n' << std::dec << i / 8 << ' ';
    std::cout << std::hex << opb_mem[i] << ' ';
  }
  std::cout << '\n';

  uint32_t param_mem[32];
  for (int i = 0; i < 32; i++) {
    param_mem[i] = 0xeeeeeeee;
  }
  isp_tsp_parameters(param_mem, q);
  for (int i = 0; i < 32; i++) {
    std::cout << std::hex << param_mem[i] << '\n';
  }
}
*/
