#include "memorymap.hpp"

#include "holly/core/object_list_bits.hpp"
#include "holly/core/region_array.hpp"
#include "holly/core/region_array_bits.hpp"
#include "holly/core/parameter_bits.hpp"
#include "holly/core/parameter.hpp"
#include "holly/holly.hpp"
#include "holly/holly_bits.hpp"

void transfer_object_list(uint32_t object_list_start, uint32_t triangle_array_offset)
{
  using namespace holly::core;

  volatile uint32_t * object_list = (volatile uint32_t *)&texture_memory32[object_list_start];

  object_list[0] = object_list::pointer_type::triangle_array
                 | object_list::triangle_array::number_of_triangles(0)
                 | object_list::triangle_array::skip(1)
                 | object_list::triangle_array::start(triangle_array_offset / 4);

  object_list[1] = object_list::pointer_type::object_pointer_block_link
                 | object_list::object_pointer_block_link::end_of_list;
}

void transfer_background_polygon(uint32_t isp_tsp_parameter_start)
{
  using namespace holly::core::parameter;

  using parameter = isp_tsp_parameter<3>;

  volatile parameter * polygon = (volatile parameter *)&texture_memory32[isp_tsp_parameter_start];

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                    | isp_tsp_instruction_word::culling_mode::no_culling;

  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog;

  polygon->texture_control_word = 0;

  polygon->vertex[0].x =  0.0f;
  polygon->vertex[0].y =  0.0f;
  polygon->vertex[0].z =  0.00001f;
  polygon->vertex[0].base_color = 0xff00ff;

  polygon->vertex[1].x = 32.0f;
  polygon->vertex[1].y =  0.0f;
  polygon->vertex[1].z =  0.00001f;
  polygon->vertex[1].base_color = 0xff00ff;

  polygon->vertex[2].x = 32.0f;
  polygon->vertex[2].y = 32.0f;
  polygon->vertex[2].z =  0.00001f;
  polygon->vertex[2].base_color = 0xff00ff;
}

void transfer_triangle_polygon(uint32_t isp_tsp_parameter_start)
{
  using namespace holly::core::parameter;

  using parameter = isp_tsp_parameter<3>;

  volatile parameter * polygon = (volatile parameter *)&texture_memory32[isp_tsp_parameter_start];

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                    | isp_tsp_instruction_word::culling_mode::no_culling
                                    | isp_tsp_instruction_word::gouraud_shading;

  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog;

  polygon->texture_control_word = 0;

  polygon->vertex[0].x =  77.0f;
  polygon->vertex[0].y = 450.0f;
  polygon->vertex[0].z =   0.1f;
  polygon->vertex[0].base_color = 0xff0000;

  polygon->vertex[1].x = 320.0f;
  polygon->vertex[1].y =  30.0f;
  polygon->vertex[1].z =   0.1f;
  polygon->vertex[1].base_color = 0x00ff00;

  polygon->vertex[2].x = 562.0f;
  polygon->vertex[2].y = 450.0f;
  polygon->vertex[2].z =   0.1f;
  polygon->vertex[2].base_color = 0x0000ff;
}


void transfer_region_array(uint32_t region_array_start,
                           uint32_t opaque_list_pointer)
{
  using namespace holly::core::region_array;
  /*
    Create a minimal region array with a single entry:
       - one tile at tile coordinate (0, 0) with one opaque list pointer
  */

  /*
    Holly reads the region array from "32-bit" texture memory address space,
    so the region array is correspondingly written from "32-bit" address space.
   */
  volatile region_array_entry * region_array = (volatile region_array_entry *)&texture_memory32[region_array_start];

  const int num_tiles_y = 480 / 32;
  const int num_tiles_x = 640 / 32;
  const int num_tiles = num_tiles_x * num_tiles_y;

  for (int i = 0; i < num_tiles; i++) {
    /* define one region array entry per 32×32 px tile over a 640x480 px area */

    int y = i / num_tiles_x;
    int x = i % num_tiles_x;

    bool last_tile = (i == (num_tiles - 1));

    region_array[i].tile
      = (last_tile ? tile::last_region : 0)
      | tile::y_position(y)
      | tile::x_position(x);

    /*
      list pointers are offsets relative to the beginning of "32-bit" texture memory.

      each list type uses different rasterization steps, "opaque" being the fastest and most efficient.
     */
    region_array[i].list_pointer.opaque                      = list_pointer::object_list(opaque_list_pointer);
    region_array[i].list_pointer.opaque_modifier_volume      = list_pointer::empty;
    region_array[i].list_pointer.translucent                 = list_pointer::empty;
    region_array[i].list_pointer.translucent_modifier_volume = list_pointer::empty;
    region_array[i].list_pointer.punch_through               = list_pointer::empty;
  }
}

void main()
{
  uint32_t framebuffer_start       = 0x200000;
  uint32_t isp_tsp_parameter_start = 0x400000;
  uint32_t region_array_start      = 0x500000;
  uint32_t object_list_start       = 0x100000;

  transfer_region_array(region_array_start,
                        object_list_start);

  using polygon = holly::core::parameter::isp_tsp_parameter<3>;

  uint32_t background_offset = (sizeof (polygon)) * 0;
  uint32_t triangle_offset   = (sizeof (polygon)) * 1;

  transfer_object_list(object_list_start,
                       triangle_offset);

  transfer_triangle_polygon(isp_tsp_parameter_start + triangle_offset);

  transfer_background_polygon(isp_tsp_parameter_start + background_offset);

  {
    using namespace holly;
    using holly::holly;

    // REGION_BASE is the (texture memory-relative) address of the region array.
    holly.REGION_BASE = region_array_start;

    // PARAM_BASE is the (texture memory-relative) address of ISP/TSP parameters.
    // Anything that references an ISP/TSP parameter does so relative to this
    // address (and not relative to the beginning of texture memory).
    holly.PARAM_BASE = isp_tsp_parameter_start;

    // Set the offset of the background ISP/TSP parameter, relative to PARAM_BASE
    // SKIP is related to the size of each vertex
    holly.ISP_BACKGND_T = isp_backgnd_t::tag_address(background_offset / 4)
                        | isp_backgnd_t::tag_offset(0)
                        | isp_backgnd_t::skip(1);

    holly.ISP_BACKGND_D = 0.000001f;

    // FB_W_SOF1 is the (texture memory-relative) address of the framebuffer that
    // will be written to when a tile is rendered/flushed.
    holly.FB_W_SOF1 = framebuffer_start;

    // start the actual render--the rendering process begins by interpreting the
    // region array
    holly.STARTRENDER = 1;

    // without waiting for rendering to actually complete, immediately display the
    // framebuffer.
    holly.FB_R_SOF1 = framebuffer_start;
  }
}
