#pragma once

#include "math.hpp"
#include "vec.hpp"

//
// vec3
//

template <typename T>
struct vec<3, T>
{
  union {
    struct { T x, y, z; };
    struct { T r, g, b; };
  };

  inline constexpr vec();
  inline constexpr vec(T scalar);
  inline constexpr vec(T _x, T _y, T _z);

  constexpr inline vec<3, T> operator-() const;
  inline constexpr T const& operator[](int i) const;
  inline constexpr vec<3, T>& operator=(vec<3, T> const& v);
  inline constexpr vec<3, T>& operator+=(vec<3, T> const& v);
  inline constexpr vec<3, T>& operator-=(vec<3, T> const& v);
};

template <typename T>
inline constexpr vec<3, T>::vec()
  : x(0), y(0), z(0)
{}

template <typename T>
inline constexpr vec<3, T>::vec(T scalar)
  : x(scalar), y(scalar), z(scalar)
{}

template <typename T>
inline constexpr vec<3, T>::vec(T _x, T _y, T _z)
  : x(_x), y(_y), z(_z)
{}

template <typename T>
constexpr inline vec<3, T> vec<3, T>::operator-() const
{
  return vec<3, T>(-x, -y, -z);
}

template <typename T>
inline constexpr T const& vec<3, T>::operator[](int i) const
{
  switch(i)
  {
  default: [[fallthrough]];
  case 0: return x;
  case 1: return y;
  case 2: return z;
  }
}

template <typename T>
inline constexpr vec<3, T>& vec<3, T>::operator=(vec<3, T> const& v)
{
  this->x = static_cast<T>(v.x);
  this->y = static_cast<T>(v.y);
  this->z = static_cast<T>(v.z);
  return *this;
}

template <typename T>
inline constexpr vec<3, T>& vec<3, T>::operator+=(vec<3, T> const& v)
{
  *this = *this + vec<3, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<3, T>& vec<3, T>::operator-=(vec<3, T> const& v)
{
  *this = *this - vec<3, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<3, T> operator+(vec<3, T> const& v1, vec<3, T> const& v2)
{
  return vec<3, T>(v1.x + v2.x,
                   v1.y + v2.y,
                   v1.z + v2.z);
}

template <typename T>
inline constexpr vec<3, T> operator-(vec<3, T> const& v1, vec<3, T> const& v2)
{
  return vec<3, T>(v1.x - v2.x,
                   v1.y - v2.y,
                   v1.z - v2.z);
}

template <typename T>
inline constexpr vec<3, T> operator*(vec<3, T> const& v1, vec<3, T> const& v2)
{
  return vec<3, T>(v1.x * v2.x,
                   v1.y * v2.y,
                   v1.z * v2.z);
}

template <typename T>
inline constexpr vec<3, T> operator*(vec<3, T> const& v1, T const& scalar)
{
  return v1 * vec<3, T>(scalar);
}

template <typename T>
inline constexpr vec<3, T> operator*(T const& scalar, vec<3, T> const& v1)
{
  return vec<3, T>(scalar) * v1;
}

template <typename T>
inline constexpr vec<3, T> operator/(vec<3, T> const& v1, vec<3, T> const& v2)
{
  return vec<3, T>(v1.x / v2.x,
                   v1.y / v2.y,
                   v1.z / v2.z);
}

template <typename T>
inline constexpr vec<3, T> operator/(vec<3, T> const& v1, T const& scalar)
{
  return v1 / vec<3, T>(scalar);
}

template <typename T>
inline constexpr T dot(vec<3, T> const& v1, vec<3, T> const& v2)
{
  vec<3, T> tmp(v1 * v2);
  return tmp.x + tmp.y + tmp.z;
}

template <typename T>
inline constexpr vec<3, T> cross(vec<3, T> const& v1, vec<3, T> const& v2)
{
  return vec<3, T>(v1.y * v2.z - v2.y * v1.z,
                   v1.z * v2.x - v2.z * v1.x,
                   v1.x * v2.y - v2.x * v1.y);
}

template <typename T>
inline constexpr vec<3, T> functor1(T (&func) (T const& x), vec<3, T> const& v)
{
  return vec<3, T>(func(v.x), func(v.y), func(v.z));
}

template <typename T, typename U>
inline constexpr vec<3, U> functor1(U (&func) (T const& x), vec<3, T> const& v)
{
  return vec<3, U>(func(v.x), func(v.y), func(v.z));
}

template <typename T>
inline constexpr T magnitude(vec<3, T> const& v)
{
  return sqrt(dot(v, v));
}
