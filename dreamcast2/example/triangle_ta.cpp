#include "memorymap.hpp"

#include "holly/core/object_list_bits.hpp"
#include "holly/core/region_array.hpp"
#include "holly/core/region_array_bits.hpp"
#include "holly/core/parameter_bits.hpp"
#include "holly/core/parameter.hpp"
#include "holly/ta/global_parameter.hpp"
#include "holly/ta/vertex_parameter.hpp"
#include "holly/ta/parameter_bits.hpp"
#include "holly/holly.hpp"
#include "holly/holly_bits.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/pref.hpp"

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

void transfer_ta_triangle()
{
  using namespace sh7091;
  using sh7091::sh7091;

  sh7091.CCN.QACR0 = sh7091::ccn::qacr0::address(ta_fifo_polygon_converter);
  sh7091.CCN.QACR1 = sh7091::ccn::qacr1::address(ta_fifo_polygon_converter);

  uint32_t store_queue_ix = 0;

  using namespace holly::core::parameter;
  using namespace holly::ta;
  using namespace holly::ta::parameter;

  //
  // TA polygon global transfer
  //

  volatile global_parameter::polygon_type_0 * polygon = (volatile global_parameter::polygon_type_0 *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (global_parameter::polygon_type_0));

  polygon->parameter_control_word = parameter_control_word::para_type::polygon_or_modifier_volume
                                  | parameter_control_word::list_type::opaque
                                  | parameter_control_word::col_type::packed_color
                                  | parameter_control_word::gouraud;

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                    | isp_tsp_instruction_word::culling_mode::no_culling;


  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog;

  pref(polygon);

  //
  // TA polygon vertex transfer
  //

  volatile vertex_parameter::polygon_type_0 * vertex = (volatile vertex_parameter::polygon_type_0 *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (vertex_parameter::polygon_type_0)) * 3;

  // bottom left
  vertex[0].parameter_control_word = parameter_control_word::para_type::vertex_parameter;
  vertex[0].x =  1.0f;
  vertex[0].y = 29.0f;
  vertex[0].z =  0.1f;
  vertex[0].base_color = 0xff0000; // red

  // start store queue transfer of `vertex[0]` to the TA
  pref(&vertex[0]);

  // top center
  vertex[1].parameter_control_word = parameter_control_word::para_type::vertex_parameter;
  vertex[1].x = 16.0f;
  vertex[1].y =  3.0f;
  vertex[1].z =  0.1f;
  vertex[1].base_color = 0x00ff00; // green

  // start store queue transfer of `vertex[1]` to the TA
  pref(&vertex[1]);

  // bottom right
  vertex[2].parameter_control_word = parameter_control_word::para_type::vertex_parameter
                                   | parameter_control_word::end_of_strip;
  vertex[2].x = 31.0f;
  vertex[2].y = 29.0f;
  vertex[2].z =  0.1f;
  vertex[2].base_color = 0x0000ff; // blue

  // start store queue transfer of `params[2]` to the TA
  pref(&vertex[2]);

  //
  // TA "end of list" global transfer
  //
  volatile global_parameter::end_of_list * end_of_list = (volatile global_parameter::end_of_list *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (global_parameter::end_of_list));

  end_of_list->parameter_control_word = parameter_control_word::para_type::end_of_list;

  // start store queue transfer of `end_of_list` to the TA
  pref(end_of_list);
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

  region_array[0].tile
    = tile::last_region
    | tile::y_position(0)
    | tile::x_position(0);

  /*
    list pointers are offsets relative to the beginning of "32-bit" texture memory.

    each list type uses different rasterization steps, "opaque" being the fastest and most efficient.
   */
  region_array[0].list_pointer.opaque                      = list_pointer::object_list(opaque_list_pointer);
  region_array[0].list_pointer.opaque_modifier_volume      = list_pointer::empty;
  region_array[0].list_pointer.translucent                 = list_pointer::empty;
  region_array[0].list_pointer.translucent_modifier_volume = list_pointer::empty;
  region_array[0].list_pointer.punch_through               = list_pointer::empty;
}

void main()
{
  /*
    a very simple memory map:

    the ordering within texture memory is not significant, and could be
    anything
  */
  uint32_t framebuffer_start       = 0x200000; // intentionally the same address that the boot rom used to draw the SEGA logo
  uint32_t isp_tsp_parameter_start = 0x400000;
  uint32_t region_array_start      = 0x500000;
  uint32_t object_list_start       = 0x100000;

  transfer_region_array(region_array_start,
                        object_list_start);

  transfer_background_polygon(isp_tsp_parameter_start);

  //////////////////////////////////////////////////////////////////////////////
  // configure the TA
  //////////////////////////////////////////////////////////////////////////////

  const int tile_y_num = 1;
  const int tile_x_num = 1;

  using namespace holly;
  using holly::holly;

  // TA_GLOB_TILE_CLIP restricts which "object pointer blocks" are written
  // to.
  //
  // This can also be used to implement "windowing", as long as the desired
  // window size happens to be a multiple of 32 pixels. The "User Tile Clip" TA
  // control parameter can also ~equivalently be used as many times as desired
  // within a single TA initialization to produce an identical effect.
  //
  // See DCDBSysArc990907E.pdf page 183.
  holly.TA_GLOB_TILE_CLIP = ta_glob_tile_clip::tile_y_num(tile_y_num - 1)
                          | ta_glob_tile_clip::tile_x_num(tile_x_num - 1);

  // While CORE supports arbitrary-length object lists, the TA uses "object
  // pointer blocks" as a memory allocation strategy. These fixed-length blocks
  // can still have infinite length via "object pointer block links". This
  // mechanism is illustrated in DCDBSysArc990907E.pdf page 188.
  holly.TA_ALLOC_CTRL = ta_alloc_ctrl::opb_mode::increasing_addresses
                      | ta_alloc_ctrl::o_opb::_8x4byte;

  // While building object lists, the TA contains an internal index (exposed as
  // the read-only TA_ITP_CURRENT) for the next address that new ISP/TSP will be
  // stored at. The initial value of this index is TA_ISP_BASE.

  // reserve space in ISP/TSP parameters for the background parameter
  using polygon = holly::core::parameter::isp_tsp_parameter<3>;
  uint32_t ta_isp_base_offset = (sizeof (polygon)) * 1;

  holly.TA_ISP_BASE = isp_tsp_parameter_start + ta_isp_base_offset;
  holly.TA_ISP_LIMIT = isp_tsp_parameter_start + 0x100000;

  // Similarly, the TA also contains, for up to 600 tiles, an internal index for
  // the next address that an object list entry will be stored for each
  // tile. These internal indicies are partially exposed via the read-only
  // TA_OL_POINTERS.
  holly.TA_OL_BASE = object_list_start;

  // TA_OL_LIMIT, DCDBSysArc990907E.pdf page 385:
  //
  // >   Because the TA may automatically store data in the address that is
  // >   specified by this register, it must not be used for other data.  For
  // >   example, the address specified here must not be the same as the address
  // >   in the TA_ISP_BASE register.
  holly.TA_OL_LIMIT = object_list_start + 0x100000 - 32;

  holly.TA_LIST_INIT = ta_list_init::list_init;

  // dummy TA_LIST_INIT read; DCDBSysArc990907E.pdf in multiple places says this
  // step is required.
  (void)holly.TA_LIST_INIT;

  //////////////////////////////////////////////////////////////////////////////
  // transfer triangles to texture memory via the TA polygon converter FIFO
  //////////////////////////////////////////////////////////////////////////////

  transfer_ta_triangle();

  //////////////////////////////////////////////////////////////////////////////
  // configure CORE
  //////////////////////////////////////////////////////////////////////////////

  // REGION_BASE is the (texture memory-relative) address of the region array.
  holly.REGION_BASE = region_array_start;

  // PARAM_BASE is the (texture memory-relative) address of ISP/TSP parameters.
  // Anything that references an ISP/TSP parameter does so relative to this
  // address (and not relative to the beginning of texture memory).
  holly.PARAM_BASE = isp_tsp_parameter_start;

  // Set the offset of the background ISP/TSP parameter, relative to PARAM_BASE
  // SKIP is related to the size of each vertex
  uint32_t background_offset = 0;

  holly.ISP_BACKGND_T = isp_backgnd_t::tag_address(background_offset / 4)
                      | isp_backgnd_t::tag_offset(0)
                      | isp_backgnd_t::skip(1);

  // FB_W_SOF1 is the (texture memory-relative) address of the framebuffer that
  // will be written to when a tile is rendered/flushed.
  holly.FB_W_SOF1 = framebuffer_start;

  // start the actual render--the rendering process begins by interpreting the
  // region array
  holly.STARTRENDER = 1;

  // without waiting for rendering to actually complete, immediately display the
  // framebuffer.
  holly.FB_R_SOF1 = framebuffer_start;

  // return from main; this will effectively jump back to the serial loader
}
