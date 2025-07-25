#pragma once

#include <utility>
#include "vec2.hpp"
#include "mat.hpp"

//
// mat2x2
//

template <typename T>
struct mat<2, 2, T>
{
  typedef vec<2, T> row_type;
  typedef vec<2, T> col_type;

private:
  row_type value[2];

public:
  inline constexpr mat();

  inline constexpr mat
  (
    T const& a00, T const& a01,
    T const& a10, T const& a11
  );

  inline static constexpr int length() { return 4; }

  inline constexpr typename mat<2, 2, T>::row_type &
  operator[](int i);

  inline constexpr typename mat<2, 2, T>::row_type const &
  operator[](int i) const;

  void operator=(const mat<2, 2, T>&) = delete;

};


template<typename T>
inline constexpr mat<2, 2, T>::mat()
  : value{std::move(row_type(1, 0)),
	  std::move(row_type(0, 1))}
{ }

template<typename T>
inline constexpr mat<2, 2, T>::mat
(
  T const& a00, T const& a01,
  T const& a10, T const& a11
)
  : value{std::move(row_type(a00, a01)),
	  std::move(row_type(a10, a11))}
{ }

template <typename T>
inline constexpr typename mat<2, 2, T>::row_type &
mat<2, 2, T>::operator[](int i)
{
  return value[i];
}

template <typename T>
inline constexpr typename mat<2, 2, T>::row_type const &
mat<2, 2, T>::operator[](int i) const
{
  return value[i];
}

template<typename T>
inline constexpr mat<2, 2, T> operator*(mat<2, 2, T> const& m1, mat<2, 2, T> const& m2)
{
#define c(i, j) (                               \
    m1[i][0] * m2[0][j]                         \
  + m1[i][1] * m2[1][j])

  return mat<2, 2, T>(c(0,0), c(0,1),
                      c(1,0), c(1,1));
#undef c
}

template<typename T>
inline constexpr typename mat<2, 2, T>::row_type operator*
(
  mat<2, 2, T> const& m,
  typename mat<2, 2, T>::col_type const& v
)
{
#define c(i) (                                  \
    m[i][0] * v[0]                              \
  + m[i][1] * v[1])

  return typename mat<2, 2, T>::row_type(c(0), c(1));
#undef c
}

template<typename T>
inline constexpr mat<2, 2, T> transpose(mat<2, 2, T> const& m)
{
  return mat<2, 2, T>(
    m[0][0], m[1][0],
    m[0][1], m[1][1]
  );
}

template<typename T>
inline constexpr float determinant(mat<2, 2, T> const& a)
{
  return a[0][0] * a[1][1] - a[0][1] * a[1][0];
}
