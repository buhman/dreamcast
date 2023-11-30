#include <cstdint>
#include <cstddef>

#include "float_uint32.h"
#include "ta_parameter.h"
#include "isp_tsp.h"

static_assert((sizeof (float)) == (sizeof (uint32_t)));

struct vertex_polygon_type_0 {
  uint32_t parameter_control_word;
  float x;
  float y;
  float z;
  uint32_t _res0;
  uint32_t _res1;
  uint32_t base_color;
  uint32_t _res2;
};

static_assert((sizeof (vertex_polygon_type_0)) == 32);
static_assert((offsetof (struct vertex_polygon_type_0, parameter_control_word)) == 0x00);
static_assert((offsetof (struct vertex_polygon_type_0, x))            == 0x04);
static_assert((offsetof (struct vertex_polygon_type_0, y))            == 0x08);
static_assert((offsetof (struct vertex_polygon_type_0, z))            == 0x0c);
static_assert((offsetof (struct vertex_polygon_type_0, _res0))        == 0x10);
static_assert((offsetof (struct vertex_polygon_type_0, _res1))        == 0x14);
static_assert((offsetof (struct vertex_polygon_type_0, base_color))   == 0x18);
static_assert((offsetof (struct vertex_polygon_type_0, _res2))        == 0x1c);

struct global_polygon_type_0 {
  uint32_t parameter_control_word;
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  uint32_t _res0;
  uint32_t _res1;
  uint32_t data_size_for_sort_dma;
  uint32_t next_address_for_sort_dma;
};

static_assert((sizeof (global_polygon_type_0)) == 32);
static_assert((offsetof (struct global_polygon_type_0, parameter_control_word))    == 0x00);
static_assert((offsetof (struct global_polygon_type_0, isp_tsp_instruction_word))  == 0x04);
static_assert((offsetof (struct global_polygon_type_0, tsp_instruction_word))      == 0x08);
static_assert((offsetof (struct global_polygon_type_0, texture_control_word))      == 0x0c);
static_assert((offsetof (struct global_polygon_type_0, _res0))                     == 0x10);
static_assert((offsetof (struct global_polygon_type_0, _res1))                     == 0x14);
static_assert((offsetof (struct global_polygon_type_0, data_size_for_sort_dma))    == 0x18);
static_assert((offsetof (struct global_polygon_type_0, next_address_for_sort_dma)) == 0x1c);

struct global_end_of_list {
  uint32_t parameter_control_word;
  uint32_t _res0;
  uint32_t _res1;
  uint32_t _res2;
  uint32_t _res3;
  uint32_t _res4;
  uint32_t _res5;
  uint32_t _res6;
};

static_assert((sizeof (global_end_of_list)) == 32);
static_assert((offsetof (struct global_end_of_list, parameter_control_word)) == 0x00);
static_assert((offsetof (struct global_end_of_list, _res0)) == 0x04);
static_assert((offsetof (struct global_end_of_list, _res1)) == 0x08);
static_assert((offsetof (struct global_end_of_list, _res2)) == 0x0c);
static_assert((offsetof (struct global_end_of_list, _res3)) == 0x10);
static_assert((offsetof (struct global_end_of_list, _res4)) == 0x14);
static_assert((offsetof (struct global_end_of_list, _res5)) == 0x18);
static_assert((offsetof (struct global_end_of_list, _res6)) == 0x1c);

namespace para_control {
  namespace para_type {
    constexpr uint32_t end_of_list                = 0 << 29;
    constexpr uint32_t user_tile_clip             = 1 << 29;
    constexpr uint32_t object_list_set            = 2 << 29;
    constexpr uint32_t polygon_or_modifier_volume = 4 << 29;
    constexpr uint32_t sprite                     = 5 << 29;
    constexpr uint32_t vertex_parameter           = 7 << 29;
  }

  constexpr uint32_t end_of_strip = 1 << 28;

  namespace list_type {
    constexpr uint32_t opaque = 0 << 24;
    constexpr uint32_t opaque_modifier_volume = 1 << 24;
    constexpr uint32_t translucent = 2 << 24;
    constexpr uint32_t translucent_modifier_volume = 3 << 24;
    constexpr uint32_t punch_through = 4 << 24;
  }
}

namespace group_control {
  constexpr uint32_t group_en = 1 << 23;

  namespace strip_len {
    constexpr uint32_t _1_strip = 0 << 18;
    constexpr uint32_t _2_strip = 1 << 18;
    constexpr uint32_t _4_strip = 2 << 18;
    constexpr uint32_t _6_strip = 3 << 18;
  }

  namespace user_clip {
    constexpr uint32_t disabled = 0 << 16;
    constexpr uint32_t inside_enable = 2 << 16;
    constexpr uint32_t outside_enable = 3 << 16;
  }
}

namespace obj_control {
  constexpr uint32_t shadow = 1 << 7;
  constexpr uint32_t volume = 1 << 6;

  namespace col_type {
    constexpr uint32_t packed_color     = 0 << 4;
    constexpr uint32_t floating_color   = 1 << 4;
    constexpr uint32_t intensity_mode_1 = 2 << 4;
    constexpr uint32_t intensity_mode_2 = 3 << 4;
  }

  constexpr uint32_t texture = 1 << 3;
  constexpr uint32_t offset = 1 << 2;
  constexpr uint32_t gouraud = 1 << 1;
  constexpr uint32_t _16bit_uv = 1 << 0;
}

void vertex(volatile uint32_t * buf,
	    const float x,
	    const float y,
	    const float z,
	    const uint32_t base_color,
	    bool end_of_strip
	    )
{
  volatile vertex_polygon_type_0 * parameter = reinterpret_cast<volatile vertex_polygon_type_0 *>(buf);

  parameter->parameter_control_word = para_control::para_type::vertex_parameter;

  if (end_of_strip)
    parameter->parameter_control_word |= para_control::end_of_strip;

  parameter->x = x;
  parameter->y = y;
  parameter->z = z;
  parameter->_res0 = 0;
  parameter->_res1 = 0;
  parameter->base_color = base_color;
  parameter->_res2 = 0;
}

void triangle(volatile uint32_t * buf)
{
  volatile global_polygon_type_0 * parameter = reinterpret_cast<volatile global_polygon_type_0 *>(buf);

  parameter->parameter_control_word = para_control::para_type::polygon_or_modifier_volume
				    | para_control::list_type::opaque
				    | obj_control::col_type::packed_color;

  parameter->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
				      | isp_tsp_instruction_word::culling_mode::no_culling;

  parameter->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
				  | tsp_instruction_word::dst_alpha_instr::zero
				  | tsp_instruction_word::fog_control::no_fog;
  parameter->texture_control_word = 0;
  parameter->_res0 = 0;
  parameter->_res1 = 0;
  parameter->data_size_for_sort_dma = 0;
  parameter->next_address_for_sort_dma = 0;
}

void end_of_list(volatile uint32_t * buf)
{
  volatile global_end_of_list * parameter = reinterpret_cast<volatile global_end_of_list *>(buf);

  parameter->parameter_control_word = para_control::para_type::end_of_list;
  parameter->_res0 = 0;
  parameter->_res1 = 0;
  parameter->_res2 = 0;
  parameter->_res3 = 0;
  parameter->_res4 = 0;
  parameter->_res5 = 0;
  parameter->_res6 = 0;
}
