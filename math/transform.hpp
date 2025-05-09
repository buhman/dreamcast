#pragma once

template<typename T>
inline constexpr mat<4, 4, T> translate(vec<3, T> t)
{
  return {
    1, 0, 0, t.x,
    0, 1, 0, t.y,
    0, 0, 1, t.z,
    0, 0, 0,   1
  };
}

template <typename T>
inline constexpr mat<4, 4, T> scale(vec<3, T> s)
{
  return {
    s.x,   0,   0, 0,
      0, s.y,   0, 0,
      0,   0, s.z, 0,
      0,   0,   0, 1
  };
}

template <template T>
inline constexpr mat<4, 4, T> rotate_x(T t)
{
  return {
    1,      0,       0, 0,
    0, cos(t), -sin(t), 0,
    0, sin(t),  cos(t), 0,
    0,      0,       0, 1
  };
}

template <typename T>
inline constexpr mat<4, 4, T> rotate_y(T t)
{
  return {
    cos(t), 0, sin(t), 0,
         0, 1,      0, 0,
   -sin(t), 0, cos(t), 0,
         0, 0,      0, 1
  };
}

template <typename T>
inline constexpr mat<4, 4, T> rotate_z(T t)
{
  return {
    cos(t), -sin(t), 0, 0,
    sin(t),  cos(t), 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

template <typename T>
inline constexpr mat<4, 4, T> rotate_axis_angle(vec<3, T> u, T t)
{
  T st = sin(t);
  T ct = cos(t);
  T oct = 1.0 - ct;

  T xx = u.x * u.x;
  T xy = u.x * u.y;
  T xz = u.x * u.z;
  T yy = u.y * u.y;
  T yz = u.z * u.z;
  T zz = u.z * u.z;

  return {
    xx * oct +       ct,   xy * oct - u.z * st,   xz * oct + u.y * st,   0,
    xy * oct + u.z * st,   yy * oct +       ct,   yz * oct - u.x * st,   0,
    xz * oct - u.y * st,   yz * oct + u.x * st,   zz * oct +       ct,   0,
                      0,                     0,                     0,   0
  };
}

template <typename T>
inline constexpr vec<3, T> normal_multiply(mat<4, 4, T> m, vec<3, T> n)
{
  vec<4, T> n4 = m * (vec<4, T>){n.x, n.y, n.z, 0.0};
  return {n4.x, n4.y, n4.z};
}

template <typename T>
inline constexpr T inverse_length(vec<3, T> v)
{
  float f = dot(v, v);
  return 1.0f / sqrt(f);
}

template <typename T>
inline constexpr T screen_transform(T x, T y)
{
  T x2 = x / 2.0;
  T y2 = y / 2.0;

  return {
    y2,  0, 0, x2,
     0, y2, 0, y2,
     0,  0, 1,  0,
     0,  0, 0,  1
  };
}
