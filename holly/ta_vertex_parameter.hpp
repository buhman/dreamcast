#pragma once

#include <cstdint>
#include <cstddef>

namespace ta_vertex_parameter {
  struct polygon_type_0 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t base_color;
    uint32_t _res2;

    constexpr polygon_type_0(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const uint32_t base_color
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , _res0(0)
      , _res1(0)
      , base_color(base_color)
      , _res2(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_0)) == 32);
  static_assert((offsetof (struct polygon_type_0, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_0, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_0, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_0, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_0, _res0)) == 0x10);
  static_assert((offsetof (struct polygon_type_0, _res1)) == 0x14);
  static_assert((offsetof (struct polygon_type_0, base_color)) == 0x18);
  static_assert((offsetof (struct polygon_type_0, _res2)) == 0x1c);

  struct polygon_type_1 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float base_color_alpha;
    float base_color_r;
    float base_color_g;
    float base_color_b;

    constexpr polygon_type_1(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const float base_color_alpha,
                             const float base_color_r,
                             const float base_color_g,
                             const float base_color_b
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , base_color_alpha(base_color_alpha)
      , base_color_r(base_color_r)
      , base_color_g(base_color_g)
      , base_color_b(base_color_b)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_1)) == 32);
  static_assert((offsetof (struct polygon_type_1, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_1, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_1, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_1, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_1, base_color_alpha)) == 0x10);
  static_assert((offsetof (struct polygon_type_1, base_color_r)) == 0x14);
  static_assert((offsetof (struct polygon_type_1, base_color_g)) == 0x18);
  static_assert((offsetof (struct polygon_type_1, base_color_b)) == 0x1c);

  struct polygon_type_2 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t _res0;
    uint32_t _res1;
    float base_intensity;
    uint32_t _res2;

    constexpr polygon_type_2(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const float base_intensity
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , _res0(0)
      , _res1(0)
      , base_intensity(base_intensity)
      , _res2(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_2)) == 32);
  static_assert((offsetof (struct polygon_type_2, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_2, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_2, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_2, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_2, _res0)) == 0x10);
  static_assert((offsetof (struct polygon_type_2, _res1)) == 0x14);
  static_assert((offsetof (struct polygon_type_2, base_intensity)) == 0x18);
  static_assert((offsetof (struct polygon_type_2, _res2)) == 0x1c);

  struct polygon_type_3 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t base_color;
    uint32_t offset_color;

    constexpr polygon_type_3(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const float u,
                             const float v,
                             const uint32_t base_color,
                             const uint32_t offset_color
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u(u)
      , v(v)
      , base_color(base_color)
      , offset_color(offset_color)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_3)) == 32);
  static_assert((offsetof (struct polygon_type_3, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_3, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_3, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_3, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_3, u)) == 0x10);
  static_assert((offsetof (struct polygon_type_3, v)) == 0x14);
  static_assert((offsetof (struct polygon_type_3, base_color)) == 0x18);
  static_assert((offsetof (struct polygon_type_3, offset_color)) == 0x1c);

  struct polygon_type_4 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t u_v;
    uint32_t _res0;
    uint32_t base_color;
    uint32_t offset_color;

    constexpr polygon_type_4(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const uint32_t u_v,
                             const uint32_t base_color,
                             const uint32_t offset_color
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_v(u_v)
      , _res0(0)
      , base_color(base_color)
      , offset_color(offset_color)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_4)) == 32);
  static_assert((offsetof (struct polygon_type_4, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_4, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_4, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_4, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_4, u_v)) == 0x10);
  static_assert((offsetof (struct polygon_type_4, _res0)) == 0x14);
  static_assert((offsetof (struct polygon_type_4, base_color)) == 0x18);
  static_assert((offsetof (struct polygon_type_4, offset_color)) == 0x1c);

  struct polygon_type_5 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t _res0;
    uint32_t _res1;
    float base_color_alpha;
    float base_color_r;
    float base_color_g;
    float base_color_b;
    float offset_color_alpha;
    float offset_color_r;
    float offset_color_g;
    float offset_color_b;

    constexpr polygon_type_5(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const float u,
                             const float v,
                             const float base_color_alpha,
                             const float base_color_r,
                             const float base_color_g,
                             const float base_color_b,
                             const float offset_color_alpha,
                             const float offset_color_r,
                             const float offset_color_g,
                             const float offset_color_b
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u(u)
      , v(v)
      , _res0(0)
      , _res1(0)
      , base_color_alpha(base_color_alpha)
      , base_color_r(base_color_r)
      , base_color_g(base_color_g)
      , base_color_b(base_color_b)
      , offset_color_alpha(offset_color_alpha)
      , offset_color_r(offset_color_r)
      , offset_color_g(offset_color_g)
      , offset_color_b(offset_color_b)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_5)) == 64);
  static_assert((offsetof (struct polygon_type_5, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_5, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_5, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_5, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_5, u)) == 0x10);
  static_assert((offsetof (struct polygon_type_5, v)) == 0x14);
  static_assert((offsetof (struct polygon_type_5, _res0)) == 0x18);
  static_assert((offsetof (struct polygon_type_5, _res1)) == 0x1c);
  static_assert((offsetof (struct polygon_type_5, base_color_alpha)) == 0x20);
  static_assert((offsetof (struct polygon_type_5, base_color_r)) == 0x24);
  static_assert((offsetof (struct polygon_type_5, base_color_g)) == 0x28);
  static_assert((offsetof (struct polygon_type_5, base_color_b)) == 0x2c);
  static_assert((offsetof (struct polygon_type_5, offset_color_alpha)) == 0x30);
  static_assert((offsetof (struct polygon_type_5, offset_color_r)) == 0x34);
  static_assert((offsetof (struct polygon_type_5, offset_color_g)) == 0x38);
  static_assert((offsetof (struct polygon_type_5, offset_color_b)) == 0x3c);

  struct polygon_type_6 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t u_v;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    float base_color_alpha;
    float base_color_r;
    float base_color_g;
    float base_color_b;
    float offset_color_alpha;
    float offset_color_r;
    float offset_color_g;
    float offset_color_b;

    constexpr polygon_type_6(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const uint32_t u_v,
                             const float base_color_alpha,
                             const float base_color_r,
                             const float base_color_g,
                             const float base_color_b,
                             const float offset_color_alpha,
                             const float offset_color_r,
                             const float offset_color_g,
                             const float offset_color_b
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_v(u_v)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , base_color_alpha(base_color_alpha)
      , base_color_r(base_color_r)
      , base_color_g(base_color_g)
      , base_color_b(base_color_b)
      , offset_color_alpha(offset_color_alpha)
      , offset_color_r(offset_color_r)
      , offset_color_g(offset_color_g)
      , offset_color_b(offset_color_b)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_6)) == 64);
  static_assert((offsetof (struct polygon_type_6, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_6, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_6, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_6, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_6, u_v)) == 0x10);
  static_assert((offsetof (struct polygon_type_6, _res0)) == 0x14);
  static_assert((offsetof (struct polygon_type_6, _res1)) == 0x18);
  static_assert((offsetof (struct polygon_type_6, _res2)) == 0x1c);
  static_assert((offsetof (struct polygon_type_6, base_color_alpha)) == 0x20);
  static_assert((offsetof (struct polygon_type_6, base_color_r)) == 0x24);
  static_assert((offsetof (struct polygon_type_6, base_color_g)) == 0x28);
  static_assert((offsetof (struct polygon_type_6, base_color_b)) == 0x2c);
  static_assert((offsetof (struct polygon_type_6, offset_color_alpha)) == 0x30);
  static_assert((offsetof (struct polygon_type_6, offset_color_r)) == 0x34);
  static_assert((offsetof (struct polygon_type_6, offset_color_g)) == 0x38);
  static_assert((offsetof (struct polygon_type_6, offset_color_b)) == 0x3c);

  struct polygon_type_7 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float u;
    float v;
    float base_intensity;
    float offset_intensity;

    constexpr polygon_type_7(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const float u,
                             const float v,
                             const float base_intensity,
                             const float offset_intensity
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u(u)
      , v(v)
      , base_intensity(base_intensity)
      , offset_intensity(offset_intensity)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_7)) == 32);
  static_assert((offsetof (struct polygon_type_7, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_7, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_7, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_7, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_7, u)) == 0x10);
  static_assert((offsetof (struct polygon_type_7, v)) == 0x14);
  static_assert((offsetof (struct polygon_type_7, base_intensity)) == 0x18);
  static_assert((offsetof (struct polygon_type_7, offset_intensity)) == 0x1c);

  struct polygon_type_8 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t u_v;
    uint32_t _res0;
    float base_intensity;
    float offset_intensity;

    constexpr polygon_type_8(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const uint32_t u_v,
                             const float base_intensity,
                             const float offset_intensity
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_v(u_v)
      , _res0(0)
      , base_intensity(base_intensity)
      , offset_intensity(offset_intensity)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_8)) == 32);
  static_assert((offsetof (struct polygon_type_8, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_8, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_8, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_8, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_8, u_v)) == 0x10);
  static_assert((offsetof (struct polygon_type_8, _res0)) == 0x14);
  static_assert((offsetof (struct polygon_type_8, base_intensity)) == 0x18);
  static_assert((offsetof (struct polygon_type_8, offset_intensity)) == 0x1c);

  struct polygon_type_9 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t base_color_0;
    uint32_t base_color_1;
    uint32_t _res0;
    uint32_t _res1;

    constexpr polygon_type_9(const uint32_t parameter_control_word,
                             const float x,
                             const float y,
                             const float z,
                             const uint32_t base_color_0,
                             const uint32_t base_color_1
                             )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , base_color_0(base_color_0)
      , base_color_1(base_color_1)
      , _res0(0)
      , _res1(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_9)) == 32);
  static_assert((offsetof (struct polygon_type_9, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_9, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_9, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_9, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_9, base_color_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_9, base_color_1)) == 0x14);
  static_assert((offsetof (struct polygon_type_9, _res0)) == 0x18);
  static_assert((offsetof (struct polygon_type_9, _res1)) == 0x1c);

  struct polygon_type_10 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t base_intensity_0;
    uint32_t base_intensity_1;
    uint32_t _res0;
    uint32_t _res1;

    constexpr polygon_type_10(const uint32_t parameter_control_word,
                              const float x,
                              const float y,
                              const float z,
                              const uint32_t base_intensity_0,
                              const uint32_t base_intensity_1
                              )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , base_intensity_0(base_intensity_0)
      , base_intensity_1(base_intensity_1)
      , _res0(0)
      , _res1(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_10)) == 32);
  static_assert((offsetof (struct polygon_type_10, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_10, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_10, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_10, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_10, base_intensity_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_10, base_intensity_1)) == 0x14);
  static_assert((offsetof (struct polygon_type_10, _res0)) == 0x18);
  static_assert((offsetof (struct polygon_type_10, _res1)) == 0x1c);

  struct polygon_type_11 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float u_0;
    float v_0;
    uint32_t base_color_0;
    uint32_t offset_color_0;
    float u_1;
    float v_1;
    uint32_t base_color_1;
    uint32_t offset_color_1;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;

    constexpr polygon_type_11(const uint32_t parameter_control_word,
                              const float x,
                              const float y,
                              const float z,
                              const float u_0,
                              const float v_0,
                              const uint32_t base_color_0,
                              const uint32_t offset_color_0,
                              const float u_1,
                              const float v_1,
                              const uint32_t base_color_1,
                              const uint32_t offset_color_1
                              )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_0(u_0)
      , v_0(v_0)
      , base_color_0(base_color_0)
      , offset_color_0(offset_color_0)
      , u_1(u_1)
      , v_1(v_1)
      , base_color_1(base_color_1)
      , offset_color_1(offset_color_1)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_11)) == 64);
  static_assert((offsetof (struct polygon_type_11, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_11, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_11, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_11, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_11, u_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_11, v_0)) == 0x14);
  static_assert((offsetof (struct polygon_type_11, base_color_0)) == 0x18);
  static_assert((offsetof (struct polygon_type_11, offset_color_0)) == 0x1c);
  static_assert((offsetof (struct polygon_type_11, u_1)) == 0x20);
  static_assert((offsetof (struct polygon_type_11, v_1)) == 0x24);
  static_assert((offsetof (struct polygon_type_11, base_color_1)) == 0x28);
  static_assert((offsetof (struct polygon_type_11, offset_color_1)) == 0x2c);
  static_assert((offsetof (struct polygon_type_11, _res0)) == 0x30);
  static_assert((offsetof (struct polygon_type_11, _res1)) == 0x34);
  static_assert((offsetof (struct polygon_type_11, _res2)) == 0x38);
  static_assert((offsetof (struct polygon_type_11, _res3)) == 0x3c);

  struct polygon_type_12 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t u_v_0;
    uint32_t _res0;
    uint32_t base_color_0;
    uint32_t offset_color_0;
    uint32_t u_v_1;
    uint32_t _res1;
    uint32_t base_color_1;
    uint32_t offset_color_1;
    uint32_t _res2;
    uint32_t _res3;
    uint32_t _res4;
    uint32_t _res5;

    constexpr polygon_type_12(const uint32_t parameter_control_word,
                              const float x,
                              const float y,
                              const float z,
                              const uint32_t u_v_0,
                              const uint32_t base_color_0,
                              const uint32_t offset_color_0,
                              const uint32_t u_v_1,
                              const uint32_t base_color_1,
                              const uint32_t offset_color_1
                              )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_v_0(u_v_0)
      , _res0(0)
      , base_color_0(base_color_0)
      , offset_color_0(offset_color_0)
      , u_v_1(u_v_1)
      , _res1(0)
      , base_color_1(base_color_1)
      , offset_color_1(offset_color_1)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_12)) == 64);
  static_assert((offsetof (struct polygon_type_12, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_12, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_12, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_12, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_12, u_v_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_12, _res0)) == 0x14);
  static_assert((offsetof (struct polygon_type_12, base_color_0)) == 0x18);
  static_assert((offsetof (struct polygon_type_12, offset_color_0)) == 0x1c);
  static_assert((offsetof (struct polygon_type_12, u_v_1)) == 0x20);
  static_assert((offsetof (struct polygon_type_12, _res1)) == 0x24);
  static_assert((offsetof (struct polygon_type_12, base_color_1)) == 0x28);
  static_assert((offsetof (struct polygon_type_12, offset_color_1)) == 0x2c);
  static_assert((offsetof (struct polygon_type_12, _res2)) == 0x30);
  static_assert((offsetof (struct polygon_type_12, _res3)) == 0x34);
  static_assert((offsetof (struct polygon_type_12, _res4)) == 0x38);
  static_assert((offsetof (struct polygon_type_12, _res5)) == 0x3c);

  struct polygon_type_13 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    float u_0;
    float v_0;
    float base_intensity_0;
    float offset_intensity_0;
    float u_1;
    float v_1;
    float base_intensity_1;
    float offset_intensity_1;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;

    constexpr polygon_type_13(const uint32_t parameter_control_word,
                              const float x,
                              const float y,
                              const float z,
                              const float u_0,
                              const float v_0,
                              const float base_intensity_0,
                              const float offset_intensity_0,
                              const float u_1,
                              const float v_1,
                              const float base_intensity_1,
                              const float offset_intensity_1
                              )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_0(u_0)
      , v_0(v_0)
      , base_intensity_0(base_intensity_0)
      , offset_intensity_0(offset_intensity_0)
      , u_1(u_1)
      , v_1(v_1)
      , base_intensity_1(base_intensity_1)
      , offset_intensity_1(offset_intensity_1)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_13)) == 64);
  static_assert((offsetof (struct polygon_type_13, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_13, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_13, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_13, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_13, u_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_13, v_0)) == 0x14);
  static_assert((offsetof (struct polygon_type_13, base_intensity_0)) == 0x18);
  static_assert((offsetof (struct polygon_type_13, offset_intensity_0)) == 0x1c);
  static_assert((offsetof (struct polygon_type_13, u_1)) == 0x20);
  static_assert((offsetof (struct polygon_type_13, v_1)) == 0x24);
  static_assert((offsetof (struct polygon_type_13, base_intensity_1)) == 0x28);
  static_assert((offsetof (struct polygon_type_13, offset_intensity_1)) == 0x2c);
  static_assert((offsetof (struct polygon_type_13, _res0)) == 0x30);
  static_assert((offsetof (struct polygon_type_13, _res1)) == 0x34);
  static_assert((offsetof (struct polygon_type_13, _res2)) == 0x38);
  static_assert((offsetof (struct polygon_type_13, _res3)) == 0x3c);

  struct polygon_type_14 {
    uint32_t parameter_control_word;
    float x;
    float y;
    float z;
    uint32_t u_v_0;
    uint32_t _res0;
    float base_intensity_0;
    float offset_intensity_0;
    uint32_t u_v_1;
    uint32_t _res1;
    float base_intensity_1;
    float offset_intensity_1;
    uint32_t _res2;
    uint32_t _res3;
    uint32_t _res4;
    uint32_t _res5;

    constexpr polygon_type_14(const uint32_t parameter_control_word,
                              const float x,
                              const float y,
                              const float z,
                              const uint32_t u_v_0,
                              const float base_intensity_0,
                              const float offset_intensity_0,
                              const uint32_t u_v_1,
                              const float base_intensity_1,
                              const float offset_intensity_1
                              )
      : parameter_control_word(parameter_control_word)
      , x(x)
      , y(y)
      , z(z)
      , u_v_0(u_v_0)
      , _res0(0)
      , base_intensity_0(base_intensity_0)
      , offset_intensity_0(offset_intensity_0)
      , u_v_1(u_v_1)
      , _res1(0)
      , base_intensity_1(base_intensity_1)
      , offset_intensity_1(offset_intensity_1)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (polygon_type_14)) == 64);
  static_assert((offsetof (struct polygon_type_14, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_14, x)) == 0x04);
  static_assert((offsetof (struct polygon_type_14, y)) == 0x08);
  static_assert((offsetof (struct polygon_type_14, z)) == 0x0c);
  static_assert((offsetof (struct polygon_type_14, u_v_0)) == 0x10);
  static_assert((offsetof (struct polygon_type_14, _res0)) == 0x14);
  static_assert((offsetof (struct polygon_type_14, base_intensity_0)) == 0x18);
  static_assert((offsetof (struct polygon_type_14, offset_intensity_0)) == 0x1c);
  static_assert((offsetof (struct polygon_type_14, u_v_1)) == 0x20);
  static_assert((offsetof (struct polygon_type_14, _res1)) == 0x24);
  static_assert((offsetof (struct polygon_type_14, base_intensity_1)) == 0x28);
  static_assert((offsetof (struct polygon_type_14, offset_intensity_1)) == 0x2c);
  static_assert((offsetof (struct polygon_type_14, _res2)) == 0x30);
  static_assert((offsetof (struct polygon_type_14, _res3)) == 0x34);
  static_assert((offsetof (struct polygon_type_14, _res4)) == 0x38);
  static_assert((offsetof (struct polygon_type_14, _res5)) == 0x3c);

  struct sprite_type_0 {
    uint32_t parameter_control_word;
    float a_x;
    float a_y;
    float a_z;
    float b_x;
    float b_y;
    float b_z;
    float c_x;
    float c_y;
    float c_z;
    float d_x;
    float d_y;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;

    constexpr sprite_type_0(const uint32_t parameter_control_word,
                            const float a_x,
                            const float a_y,
                            const float a_z,
                            const float b_x,
                            const float b_y,
                            const float b_z,
                            const float c_x,
                            const float c_y,
                            const float c_z,
                            const float d_x,
                            const float d_y
                            )
      : parameter_control_word(parameter_control_word)
      , a_x(a_x)
      , a_y(a_y)
      , a_z(a_z)
      , b_x(b_x)
      , b_y(b_y)
      , b_z(b_z)
      , c_x(c_x)
      , c_y(c_y)
      , c_z(c_z)
      , d_x(d_x)
      , d_y(d_y)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (sprite_type_0)) == 64);
  static_assert((offsetof (struct sprite_type_0, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct sprite_type_0, a_x)) == 0x04);
  static_assert((offsetof (struct sprite_type_0, a_y)) == 0x08);
  static_assert((offsetof (struct sprite_type_0, a_z)) == 0x0c);
  static_assert((offsetof (struct sprite_type_0, b_x)) == 0x10);
  static_assert((offsetof (struct sprite_type_0, b_y)) == 0x14);
  static_assert((offsetof (struct sprite_type_0, b_z)) == 0x18);
  static_assert((offsetof (struct sprite_type_0, c_x)) == 0x1c);
  static_assert((offsetof (struct sprite_type_0, c_y)) == 0x20);
  static_assert((offsetof (struct sprite_type_0, c_z)) == 0x24);
  static_assert((offsetof (struct sprite_type_0, d_x)) == 0x28);
  static_assert((offsetof (struct sprite_type_0, d_y)) == 0x2c);
  static_assert((offsetof (struct sprite_type_0, _res0)) == 0x30);
  static_assert((offsetof (struct sprite_type_0, _res1)) == 0x34);
  static_assert((offsetof (struct sprite_type_0, _res2)) == 0x38);
  static_assert((offsetof (struct sprite_type_0, _res3)) == 0x3c);

  struct sprite_type_1 {
    uint32_t parameter_control_word;
    float a_x;
    float a_y;
    float a_z;
    float b_x;
    float b_y;
    float b_z;
    float c_x;
    float c_y;
    float c_z;
    float d_x;
    float d_y;
    uint32_t _res0;
    uint32_t a_u_a_v;
    uint32_t b_u_b_v;
    uint32_t c_u_c_v;

    constexpr sprite_type_1(const uint32_t parameter_control_word,
                            const float a_x,
                            const float a_y,
                            const float a_z,
                            const float b_x,
                            const float b_y,
                            const float b_z,
                            const float c_x,
                            const float c_y,
                            const float c_z,
                            const float d_x,
                            const float d_y,
                            const uint32_t a_u_a_v,
                            const uint32_t b_u_b_v,
                            const uint32_t c_u_c_v
                            )
      : parameter_control_word(parameter_control_word)
      , a_x(a_x)
      , a_y(a_y)
      , a_z(a_z)
      , b_x(b_x)
      , b_y(b_y)
      , b_z(b_z)
      , c_x(c_x)
      , c_y(c_y)
      , c_z(c_z)
      , d_x(d_x)
      , d_y(d_y)
      , _res0(0)
      , a_u_a_v(a_u_a_v)
      , b_u_b_v(b_u_b_v)
      , c_u_c_v(c_u_c_v)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (sprite_type_1)) == 64);
  static_assert((offsetof (struct sprite_type_1, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct sprite_type_1, a_x)) == 0x04);
  static_assert((offsetof (struct sprite_type_1, a_y)) == 0x08);
  static_assert((offsetof (struct sprite_type_1, a_z)) == 0x0c);
  static_assert((offsetof (struct sprite_type_1, b_x)) == 0x10);
  static_assert((offsetof (struct sprite_type_1, b_y)) == 0x14);
  static_assert((offsetof (struct sprite_type_1, b_z)) == 0x18);
  static_assert((offsetof (struct sprite_type_1, c_x)) == 0x1c);
  static_assert((offsetof (struct sprite_type_1, c_y)) == 0x20);
  static_assert((offsetof (struct sprite_type_1, c_z)) == 0x24);
  static_assert((offsetof (struct sprite_type_1, d_x)) == 0x28);
  static_assert((offsetof (struct sprite_type_1, d_y)) == 0x2c);
  static_assert((offsetof (struct sprite_type_1, _res0)) == 0x30);
  static_assert((offsetof (struct sprite_type_1, a_u_a_v)) == 0x34);
  static_assert((offsetof (struct sprite_type_1, b_u_b_v)) == 0x38);
  static_assert((offsetof (struct sprite_type_1, c_u_c_v)) == 0x3c);

  struct modifier_volume {
    uint32_t parameter_control_word;
    float a_x;
    float a_y;
    float a_z;
    float b_x;
    float b_y;
    float b_z;
    float c_x;
    float c_y;
    float c_z;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;
    uint32_t _res4;
    uint32_t _res5;

    constexpr modifier_volume(const uint32_t parameter_control_word,
                              const float a_x,
                              const float a_y,
                              const float a_z,
                              const float b_x,
                              const float b_y,
                              const float b_z,
                              const float c_x,
                              const float c_y,
                              const float c_z
                              )
      : parameter_control_word(parameter_control_word)
      , a_x(a_x)
      , a_y(a_y)
      , a_z(a_z)
      , b_x(b_x)
      , b_y(b_y)
      , b_z(b_z)
      , c_x(c_x)
      , c_y(c_y)
      , c_z(c_z)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (modifier_volume)) == 64);
  static_assert((offsetof (struct modifier_volume, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct modifier_volume, a_x)) == 0x04);
  static_assert((offsetof (struct modifier_volume, a_y)) == 0x08);
  static_assert((offsetof (struct modifier_volume, a_z)) == 0x0c);
  static_assert((offsetof (struct modifier_volume, b_x)) == 0x10);
  static_assert((offsetof (struct modifier_volume, b_y)) == 0x14);
  static_assert((offsetof (struct modifier_volume, b_z)) == 0x18);
  static_assert((offsetof (struct modifier_volume, c_x)) == 0x1c);
  static_assert((offsetof (struct modifier_volume, c_y)) == 0x20);
  static_assert((offsetof (struct modifier_volume, c_z)) == 0x24);
  static_assert((offsetof (struct modifier_volume, _res0)) == 0x28);
  static_assert((offsetof (struct modifier_volume, _res1)) == 0x2c);
  static_assert((offsetof (struct modifier_volume, _res2)) == 0x30);
  static_assert((offsetof (struct modifier_volume, _res3)) == 0x34);
  static_assert((offsetof (struct modifier_volume, _res4)) == 0x38);
  static_assert((offsetof (struct modifier_volume, _res5)) == 0x3c);

}
