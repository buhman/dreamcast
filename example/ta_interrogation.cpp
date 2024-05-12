#include <cstdint>

#include "align.hpp"
#include "memorymap.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/video_output.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

struct vertex {
  float x;
  float y;
  float z;
};

// screen space coordinates
const struct vertex triangles[1][3] = {
  {
    { 200.f,  360.f, 0.1f },
    { 200.f,  120.f, 0.1f },
    { 440.f,  120.f, 0.1f },
  },
};

void transform()
{
  /*
   * QACR0 [4:2]: External address bits [28:26] corresponding to SQ0
   * QACR1 [4:2]: External address bits [28:26] corresponding to SQ1
   */

  /* PREF instruction:
   *
   * [31:26]: 111000   Store queue specification
   * [25:6] : Address  External memory address bits [25:6]
   * [5]    : 0/1      0: SQ0 specification
   *                   1: SQ1 specification and external memory address bit [5]
   * [4:2]  :          No meaning in a prefetch
   * [1:0]  :          Fixed at 0
   */

  /* Store queue writes:
   * [31:26]: 111000     Store queue specification
   * [25:6] : Don’t care Used for external memory transfer/access right
   * [5]    : 0/1        0: SQ0 specification
   *                     1: SQ1 specification
   * [4:2]  :            Specifies longword position in SQ0/SQ1
   * [1:0]  :            Fixed at 0
   */

  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;

  constexpr uint32_t base_color = 0xfff0f000;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  sq_transfer_32byte(ta_fifo_polygon_converter);

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    float x = triangles[0][i].x;
    float y = triangles[0][i].y;
    float z = triangles[0][i].z;

    bool end_of_strip = i == strip_length - 1;
    *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
      ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          base_color
                                          );

    sq_transfer_32byte(ta_fifo_polygon_converter);
  }

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void transform2(ta_parameter_writer& parameter)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;

  constexpr uint32_t base_color = 0xff0f00ff;

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    float x = triangles[0][i].x;
    float y = triangles[0][i].y;
    float z = triangles[0][i].z;

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_0>() =
      ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          base_color
                                          );
  }

  parameter.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff220000);
}

uint32_t _ta_parameter_buf[((32 * (10 + 2)) + 32) / 4];

void ta_copy(uint32_t * address, uint32_t size)
{
  uint64_t * src = (uint64_t*)address;
  uint64_t dst[size / 4];

  //"Available only when PR=0 and SZ=1"
  uint32_t fpscr = (1 << 20);
  asm volatile
    ("lds %0,fpscr;"
     :
     : "r" (fpscr)
     : "memory");

  for (uint32_t i = 0; i < (size / 8); i++) {
    //uint64_t * dst = (uint64_t*)ta_fifo_polygon_converter;
    asm volatile
      ("fmov @%0,xd0;"
       "fmov xd0,@%1;"
       :                       // output
       : "r" (&src[i]), "r"(&dst[0]) // input
       : "memory");
  }

  uint32_t * src32 = (uint32_t *)&src[0];
  uint32_t * dst32 = (uint32_t *)&dst[0];

  for (uint32_t i = 0; i < (size / 4); i++) {
    serial::integer<uint32_t>(src32[i], ' ');
    serial::integer<uint32_t>(dst32[i]);
  }
  //serial::string("done\n");
}

void main()
{
  video_output::set_mode_vga();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
                                       , .translucent = 0
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;

  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);
  auto parameter = ta_parameter_writer(ta_parameter_buf);

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);
    transform2(parameter);
    serial::string("BCR1 ");
    serial::integer<uint32_t>(sh7091.BSC.BCR1);
    serial::string("BCR2 ");
    serial::integer<uint32_t>(sh7091.BSC.BCR2);
    serial::string("MCR ");
    serial::integer<uint32_t>(sh7091.BSC.MCR);
    //break;
    ta_copy(ta_parameter_buf, parameter.offset);
    break;

    ta_wait_opaque_list();
    serial::string("opaque\n");

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;;

    if (frame_ix > 10)
      break;
  }
}
