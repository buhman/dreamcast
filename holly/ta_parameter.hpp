#include <cstdint>
#include <cstddef>

#include "../float_uint32.hpp"
#include "isp_tsp.hpp"

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

  vertex_polygon_type_0(const float x,
                        const float y,
                        const float z,
                        const uint32_t base_color,
                        const bool end_of_strip
                        )
    : parameter_control_word( para_control::para_type::vertex_parameter
			    | (end_of_strip ? para_control::end_of_strip : 0)
                            )
    , x(x)
    , y(y)
    , z(z)
    , _res0(0)
    , _res1(0)
    , base_color(base_color)
    , _res2(0)
    { }
};

struct vertex_polygon_type_3 {
  uint32_t parameter_control_word;
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t base_color;
  uint32_t offset_color;

  vertex_polygon_type_3(const float x,
                        const float y,
                        const float z,
                        const float u,
                        const float v,
                        const uint32_t base_color,
                        const bool end_of_strip
                        )
    : parameter_control_word( para_control::para_type::vertex_parameter
			    | (end_of_strip ? para_control::end_of_strip : 0)
			    )
    , x(x)
    , y(y)
    , z(z)
    , u(u)
    , v(v)
    , base_color(base_color)
    , offset_color(0)
    { }
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

struct vertex_polygon_type_4 {
  uint32_t parameter_control_word;
  float x;
  float y;
  float z;
  uint32_t uv;
  uint32_t _res0;
  uint32_t base_color;
  uint32_t offset_color;

  vertex_polygon_type_4(const float x,
                        const float y,
                        const float z,
                        const uint32_t uv,
                        const uint32_t base_color,
                        const bool end_of_strip
                        )
    : parameter_control_word( para_control::para_type::vertex_parameter
			    | (end_of_strip ? para_control::end_of_strip : 0)
			    )
    , x(x)
    , y(y)
    , z(z)
    , uv(uv)
    , _res0(0)
    , base_color(base_color)
    , offset_color(0)
    { }
};

struct global_polygon_type_0 {
  uint32_t parameter_control_word;
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  uint32_t _res0;
  uint32_t _res1;
  uint32_t data_size_for_sort_dma;
  uint32_t next_address_for_sort_dma;

  // untextured
  global_polygon_type_0()
    : parameter_control_word( para_control::para_type::polygon_or_modifier_volume
                            | obj_control::col_type::packed_color )

    , isp_tsp_instruction_word( isp_tsp_instruction_word::depth_compare_mode::always
			      | isp_tsp_instruction_word::culling_mode::no_culling )

    , tsp_instruction_word( tsp_instruction_word::src_alpha_instr::src_alpha
			  | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
			  | tsp_instruction_word::fog_control::no_fog )

    , texture_control_word( 0 )

    , _res0(0)
    , _res1(0)
    , data_size_for_sort_dma(0)
    , next_address_for_sort_dma(0)
  { }

  // textured
  global_polygon_type_0(const uint32_t texture_address)
    : parameter_control_word( para_control::para_type::polygon_or_modifier_volume
                            | para_control::list_type::opaque
                            | obj_control::col_type::packed_color
                            | obj_control::texture )

    , isp_tsp_instruction_word( isp_tsp_instruction_word::depth_compare_mode::greater
			      | isp_tsp_instruction_word::culling_mode::no_culling )

    // <Note> Because a value of "0.0" is invalid for [MIP-Map] D [adjust], it must not be specified.
    , tsp_instruction_word( tsp_instruction_word::src_alpha_instr::one
			  | tsp_instruction_word::dst_alpha_instr::zero
			  | tsp_instruction_word::fog_control::no_fog
			//| tsp_instruction_word::mip_map_d_adjust(0b0100) // 1.0 (2.2 fixed-point)
			//| tsp_instruction_word::filter_mode::bilinear_filter
			//| tsp_instruction_word::clamp_uv::uv
			//| tsp_instruction_word::flip_uv::uv
			  | tsp_instruction_word::texture_u_size::_128   // 128px
			  | tsp_instruction_word::texture_v_size::_128 ) // 128px

    , texture_control_word( texture_control_word::pixel_format::_565
			  | texture_control_word::scan_order::non_twiddled
			  | texture_control_word::texture_address(texture_address / 8) )

    , _res0(0)
    , _res1(0)
    , data_size_for_sort_dma(0)
    , next_address_for_sort_dma(0)
  { if ((texture_address & 63) != 0) { while (1); } }
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

struct global_sprite {
  uint32_t parameter_control_word;
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  uint32_t base_color;
  uint32_t offset_color;
  uint32_t data_size_for_sort_dma;
  uint32_t next_address_for_sort_dma;

  global_sprite(const uint32_t base_color)
    : parameter_control_word( para_control::para_type::sprite
                            | para_control::list_type::opaque
			    | obj_control::col_type::packed_color )
    , isp_tsp_instruction_word( isp_tsp_instruction_word::depth_compare_mode::always
                              | isp_tsp_instruction_word::culling_mode::no_culling )
    , tsp_instruction_word( tsp_instruction_word::src_alpha_instr::one
                          | tsp_instruction_word::dst_alpha_instr::zero
                          | tsp_instruction_word::fog_control::no_fog)
    , texture_control_word(0)
    , base_color(base_color)
    , offset_color(0)
    , data_size_for_sort_dma(0)
    , next_address_for_sort_dma(0)
  { }
};

static_assert((sizeof (global_sprite)) == 32);

struct vertex_sprite_type_0 {
  uint32_t parameter_control_word;
  float ax;
  float ay;
  float az;
  float bx;
  float by;
  float bz;
  float cx;

  float cy;
  float cz;
  float dx;
  float dy;
  float _res0;
  float _res1;
  float _res2;
  float _res3;

  vertex_sprite_type_0(const float ax,
                       const float ay,
                       const float az,
                       const float bx,
                       const float by,
                       const float bz,
                       const float cx,
                       const float cy,
                       const float cz,
                       const float dx,
                       const float dy)
    : parameter_control_word(para_control::para_type::vertex_parameter)
    , ax(ax)
    , ay(ay)
    , az(az)
    , bx(bx)
    , by(by)
    , bz(bz)
    , cx(cx)
    , cy(cy)
    , dx(dx)
    , dy(dy)
    , _res0(0)
    , _res1(0)
    , _res2(0)
    , _res3(0)
  {}
};

static_assert((sizeof (vertex_sprite_type_0)) == 64);

struct vertex_sprite_type_1 {
  uint32_t parameter_control_word;
  float ax;
  float ay;
  float az;
  float bx;
  float by;
  float bz;
  float cx;

  float cy;
  float cz;
  float dx;
  float dy;
  float _res0;
  uint32_t au_av;
  uint32_t bu_bv;
  uint32_t cu_cv;

  vertex_sprite_type_1(const float ax,
                       const float ay,
                       const float az,
                       const float bx,
                       const float by,
                       const float bz,
                       const float cx,
                       const float cy,
                       const float cz,
                       const float dx,
                       const float dy,
		       const uint32_t au_av,
		       const uint32_t bu_bv,
		       const uint32_t cu_cv)
    : parameter_control_word(para_control::para_type::vertex_parameter)
    , ax(ax)
    , ay(ay)
    , az(az)
    , bx(bx)
    , by(by)
    , bz(bz)
    , cx(cx)
    , cy(cy)
    , dx(dx)
    , dy(dy)
    , _res0(0)
    , au_av(au_av)
    , bu_bv(bu_bv)
    , cu_cv(cu_cv)
  {}
};

static_assert((sizeof (vertex_sprite_type_1)) == 64);

struct global_end_of_list {
  uint32_t parameter_control_word;
  uint32_t _res0;
  uint32_t _res1;
  uint32_t _res2;
  uint32_t _res3;
  uint32_t _res4;
  uint32_t _res5;
  uint32_t _res6;

  global_end_of_list()
    : parameter_control_word(para_control::para_type::end_of_list)
    , _res0(0)
    , _res1(0)
    , _res2(0)
    , _res3(0)
    , _res4(0)
    , _res5(0)
    , _res6(0)
  { }
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


struct ta_parameter_writer {
  uint32_t * buf;
  uint32_t offset; // in bytes

  ta_parameter_writer(uint32_t * buf)
    : buf(buf), offset(0)
  { }

  template <typename T>
  inline T& append()
  {
    T& t = *reinterpret_cast<T *>(&buf[offset / 4]);
    offset += (sizeof (T));
    return t;
  }
};
/*
union ta_parameter {
  struct global_polygon_type_0 global_polygon_type_0;
  struct global_sprite global_sprite;

  struct vertex_polygon_type_0 vertex_polygon_type_0;
  struct vertex_polygon_type_3 vertex_polygon_type_3;

  struct vertex_sprite_type_0 vertex_sprite_type_0;

  struct global_end_of_list global_end_of_list;
};
static_assert((sizeof (ta_parameter)) == 32);
*/

uint32_t uv_16bit(float u, float v)
{
  uint32_t * ui = (reinterpret_cast<uint32_t *>(&u));
  uint32_t * vi = (reinterpret_cast<uint32_t *>(&v));
  uint32_t u_half = ((*ui) >> 16) & 0xffff;
  uint32_t v_half = ((*vi) >> 16) & 0xffff;
  return (u_half << 16) | (v_half << 0);
}
