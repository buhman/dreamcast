#pragma once

#include "math.hpp"
#include "vec.hpp"

//
// vec3
//

template <typename T>
struct vec<2, T>
{
  union {
    struct { T x, y; };
    struct { T u, v; };
  };

  inline constexpr vec();
  inline constexpr vec(T scalar);
  inline constexpr vec(T _x, T _y);

  constexpr inline vec<2, T> operator-() const;
  inline constexpr T const& operator[](int i) const;
  inline constexpr vec<2, T>& operator=(vec<2, T> const& v);
  inline constexpr vec<2, T>& operator+=(vec<2, T> const& v);
  inline constexpr vec<2, T>& operator-=(vec<2, T> const& v);
};

template <typename T>
inline constexpr vec<2, T>::vec()
  : x(0), y(0)
{}

template <typename T>
inline constexpr vec<2, T>::vec(T scalar)
  : x(scalar), y(scalar)
{}

template <typename T>
inline constexpr vec<2, T>::vec(T _x, T _y)
  : x(_x), y(_y)
{}

template <typename T>
constexpr inline vec<2, T> vec<2, T>::operator-() const
{
  return vec<2, T>(-x, -y);
}

template <typename T>
inline constexpr T const& vec<2, T>::operator[](int i) const
{
  switch(i)
  {
  default: [[fallthrough]];
  case 0: return x;
  case 1: return y;
  }
}

template <typename T>
inline constexpr vec<2, T>& vec<2, T>::operator=(vec<2, T> const& v)
{
  this->x = static_cast<T>(v.x);
  this->y = static_cast<T>(v.y);
  return *this;
}

template <typename T>
inline constexpr vec<2, T>& vec<2, T>::operator+=(vec<2, T> const& v)
{
  *this = *this + vec<2, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<2, T>& vec<2, T>::operator-=(vec<2, T> const& v)
{
  *this = *this - vec<2, T>(v);
  return *this;
}

template <typename T>
inline constexpr vec<2, T> operator+(vec<2, T> const& v1, vec<2, T> const& v2)
{
  return vec<2, T>(v1.x + v2.x,
                   v1.y + v2.y);
}

template <typename T>
inline constexpr vec<2, T> operator-(vec<2, T> const& v1, vec<2, T> const& v2)
{
  return vec<2, T>(v1.x - v2.x,
                   v1.y - v2.y);
}

template <typename T>
inline constexpr vec<2, T> operator*(vec<2, T> const& v1, vec<2, T> const& v2)
{
  return vec<2, T>(v1.x * v2.x,
                   v1.y * v2.y);
}

template <typename T>
inline constexpr vec<2, T> operator*(vec<2, T> const& v1, T const& scalar)
{
  return v1 * vec<2, T>(scalar);
}

template <typename T>
inline constexpr vec<2, T> operator*(T const& scalar, vec<2, T> const& v1)
{
  return vec<2, T>(scalar) * v1;
}

template <typename T>
inline constexpr vec<2, T> operator/(vec<2, T> const& v1, vec<2, T> const& v2)
{
  return vec<2, T>(v1.x / v2.x,
                   v1.y / v2.y);
}

template <typename T>
inline constexpr vec<2, T> operator/(vec<2, T> const& v1, T const& scalar)
{
  return v1 / vec<2, T>(scalar);
}

template <typename T>
inline constexpr T dot(vec<2, T> const& v1, vec<2, T> const& v2)
{
  vec<2, T> tmp(v1 * v2);
  return tmp.x + tmp.y;
}

template <typename T>
inline constexpr T cross(vec<2, T> const& v1, vec<2, T> const& v2)
{
  return v1.x * v2.y - v2.x * v1.y;
}

template <typename T>
inline constexpr vec<2, T> functor1(T (&func) (T const& x), vec<2, T> const& v)
{
  return vec<2, T>(func(v.x), func(v.y));
}

template <typename T, typename U>
inline constexpr vec<2, U> functor1(U (&func) (T const& x), vec<2, T> const& v)
{
  return vec<2, U>(func(v.x), func(v.y));
}

template <typename T>
inline constexpr T magnitude(vec<2, T> const& v)
{
  return sqrt(dot(v, v));
}
