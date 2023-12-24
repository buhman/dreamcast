#pragma once

#include "math.hpp"
#include "vec.hpp"

//
// vec4
//

template <typename T>
struct vec<4, T>
{
  T x, y, z, w;

  inline constexpr vec();
  inline constexpr vec(T scalar);
  inline constexpr vec(T _x, T _y, T _z, T _w);

  constexpr inline vec<4, T> operator-() const;
  inline constexpr T const& operator[](int i) const;
  inline constexpr vec<4, T>& operator=(vec<4, T> const& v);
  inline constexpr vec<4, T>& operator+=(vec<4, T> const& v);
  inline constexpr vec<4, T>& operator-=(vec<4, T> const& v);
};

template <typename T>
inline constexpr vec<4, T>::vec()
  : x(0), y(0), z(0), w(0)
{}

template <typename T>
inline constexpr vec<4, T>::vec(T scalar)
  : x(scalar), y(scalar), z(scalar), w(scalar)
{}

template <typename T>
inline constexpr vec<4, T>::vec(T _x, T _y, T _z, T _w)
  : x(_x), y(_y), z(_z), w(_w)
{}

template <typename T>
constexpr inline vec<4, T> vec<4, T>::operator-() const
{
  return vec<4, T>(-x, -y, -z, -w);
}

template <typename T>
inline constexpr T const& vec<4, T>::operator[](int i) const
{
  switch(i)
  {
  default: [[fallthrough]];
  case 0: return x;
  case 1: return y;
  case 2: return z;
  case 3: return w;
  }
}

template <typename T>
inline constexpr vec<4, T>& vec<4, T>::operator=(vec<4, T> const& v)
{
  this->x = static_cast<T>(v.x);
  this->y = static_cast<T>(v.y);
  this->z = static_cast<T>(v.z);
  this->w = static_cast<T>(v.w);
  return *this;
}

template <typename T>
inline constexpr vec<4, T>& vec<4, T>::operator+=(vec<4, T> const& v)
{
  *this = *this + vec<4, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<4, T>& vec<4, T>::operator-=(vec<4, T> const& v)
{
  *this = *this - vec<4, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<4, T> operator+(vec<4, T> const& v1, vec<4, T> const& v2)
{
  return vec<4, T>(v1.x + v2.x,
                   v1.y + v2.y,
                   v1.z + v2.z,
		   v1.w + v2.w);
}

template <typename T>
inline constexpr vec<4, T> operator-(vec<4, T> const& v1, vec<4, T> const& v2)
{
  return vec<4, T>(v1.x - v2.x,
                   v1.y - v2.y,
                   v1.z - v2.z,
		   v1.w - v2.w);
}

template <typename T>
inline constexpr vec<4, T> operator*(vec<4, T> const& v1, vec<4, T> const& v2)
{
  return vec<4, T>(v1.x * v2.x,
                   v1.y * v2.y,
                   v1.z * v2.z,
		   v1.w * v2.w);
}

template <typename T>
inline constexpr vec<4, T> operator*(vec<4, T> const& v1, T const& scalar)
{
  return v1 * vec<4, T>(scalar);
}

template <typename T>
inline constexpr vec<4, T> operator/(vec<4, T> const& v1, vec<4, T> const& v2)
{
  return vec<4, T>(v1.x / v2.x,
                   v1.y / v2.y,
                   v1.z / v2.z,
		   v1.w / v2.w);
}

template <typename T>
inline constexpr vec<4, T> operator/(vec<4, T> const& v1, T const& scalar)
{
  return v1 / vec<4, T>(scalar);
}

template <typename T>
inline constexpr T dot(vec<4, T> const& v1, vec<4, T> const& v2)
{
  vec<4, T> tmp(v1 * v2);
  return tmp.x + tmp.y + tmp.z + tmp.w;
}

template <typename T>
inline constexpr vec<4, T> functor1(T (&func) (T const& x), vec<4, T> const& v)
{
  return vec<4, T>(func(v.x), func(v.y), func(v.z), func(v.w));
}

template <typename T, typename U>
inline constexpr vec<4, U> functor1(U (&func) (T const& x), vec<4, T> const& v)
{
  return vec<4, U>(func(v.x), func(v.y), func(v.z), func(v.w));
}

template <typename T>
inline constexpr T length(vec<4, T> const& v)
{
  return sqrt(dot(v, v));
}
