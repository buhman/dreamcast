#pragma once

#include <cstdint>

namespace holly::core::parameter {

  enum struct volume_sel {
    one_volume,
    two_volumes
  };

  using volume_sel::one_volume;
  using volume_sel::two_volumes;

  enum struct offset_sel {
    offset,
    non_offset
  };

  using offset_sel::offset;
  using offset_sel::non_offset;

  enum struct texture_sel {
    texture,
    non_texture
  };

  using texture_sel::texture;
  using texture_sel::non_texture;

  template <texture_sel T, volume_sel V, offset_sel O>
  struct vertex;

  template <>
  struct vertex<non_texture, one_volume, non_offset> {
    float x;
    float y;
    float z;
    uint32_t base_color;
  };

  template <>
  struct vertex<non_texture, two_volumes, non_offset> {
    float x;
    float y;
    float z;
    struct {
      uint32_t base_color;
    } volume[2];
  };

  template <>
  struct vertex<non_texture, one_volume, offset> {
    float x;
    float y;
    float z;
    uint32_t base_color;
    uint32_t offset_color;
  };

  template <>
  struct vertex<non_texture, two_volumes, offset> {
    float x;
    float y;
    float z;
    struct {
      uint32_t base_color;
      uint32_t offset_color;
    } volume[2];
  };

  template <>
  struct vertex<texture, one_volume, non_offset> {
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t base_color;
  };

  template <>
  struct vertex<texture, two_volumes, non_offset> {
    float x;
    float y;
    float z;
    struct {
      float u;
      float v;
      uint32_t base_color;
    } volume[2];
  };

  template <>
  struct vertex<texture, one_volume, offset> {
    float x;
    float y;
    float z;
    float u;
    float v;
    uint32_t base_color;
    uint32_t offset_color;
  };

  template <>
  struct vertex<texture, two_volumes, offset> {
    float x;
    float y;
    float z;
    struct {
      float u;
      float v;
      uint32_t base_color;
      uint32_t offset_color;
    } volume[2];
  };

  template <int N, texture_sel T=non_texture, volume_sel V=one_volume, offset_sel O=non_offset>
  struct isp_tsp_parameter;

  template <int N, texture_sel T, offset_sel O>
  struct isp_tsp_parameter<N, T, one_volume, O> {
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word;
    uint32_t texture_control_word;
    struct vertex<T, one_volume, O> vertex[N];
  };

  template <int N, texture_sel T, offset_sel O>
  struct isp_tsp_parameter<N, T, two_volumes, O> {
    uint32_t isp_tsp_instruction_word;
    struct {
      uint32_t tsp_instruction_word;
      uint32_t texture_control_word;
    } volume[2];
    struct vertex<T, two_volumes, O> vertex[N];
  };

  static_assert((sizeof (isp_tsp_parameter<1>)) == 28);
  static_assert((sizeof (isp_tsp_parameter<1, texture>)) == 36);
  static_assert((sizeof (isp_tsp_parameter<1, texture, two_volumes>)) == 56);
  static_assert((sizeof (isp_tsp_parameter<1, texture, two_volumes, offset>)) == 64);

  static_assert((sizeof (isp_tsp_parameter<1, non_texture, two_volumes>)) == 40);
  static_assert((sizeof (isp_tsp_parameter<1, non_texture, two_volumes, offset>)) == 48);
}
