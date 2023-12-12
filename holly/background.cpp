#include <cstdint>

#include "isp_tsp.hpp"

struct vertex_parameter {
  float x;
  float y;
  float z;
  uint32_t base_color;
}; // ISP_BACKGND_T skip(1)

struct isp_tsp_parameter {
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  vertex_parameter vertex[3];
};

static_assert((sizeof (isp_tsp_parameter)) == (4 * 3 + 3) * 4);

void background_parameter(volatile uint32_t * buf)
{
  volatile isp_tsp_parameter * parameter = reinterpret_cast<volatile isp_tsp_parameter *>(buf);

  parameter->isp_tsp_instruction_word
    = isp_tsp_instruction_word::depth_compare_mode::always
    | isp_tsp_instruction_word::culling_mode::no_culling;

  parameter->tsp_instruction_word
    = tsp_instruction_word::src_alpha_instr::one
    | tsp_instruction_word::dst_alpha_instr::zero
    | tsp_instruction_word::fog_control::no_fog;

  parameter->texture_control_word
    = 0;

  parameter->vertex[0].x = 0.f;
  parameter->vertex[0].y = 0.f;
  parameter->vertex[0].z = 1.f/100000;
  parameter->vertex[0].base_color = 0xff0000ff;

  parameter->vertex[1].x = 639.f;
  parameter->vertex[1].y = 0.f;
  parameter->vertex[1].z = 1.f/100000;
  parameter->vertex[1].base_color = 0xff0000ff;

  parameter->vertex[2].x = 639.f;
  parameter->vertex[2].y = 479.f;
  parameter->vertex[2].z = 1.f/100000;
  parameter->vertex[2].base_color = 0xff0000ff;
}
