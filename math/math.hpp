#pragma once

template <typename T>
constexpr T sqrt(const T n) noexcept;

template <>
constexpr float sqrt(const float n) noexcept
{
  return __builtin_sqrtf(n);
}

template <typename T>
constexpr T cos(const T n) noexcept;

template <>
constexpr float cos(const float n) noexcept
{
  return __builtin_cosf(n);
}

template <typename T>
constexpr T sin(const T n) noexcept;

template <>
constexpr float sin(const float n) noexcept
{
  return __builtin_sinf(n);
}

constexpr float pi = 3.141592653589793;
