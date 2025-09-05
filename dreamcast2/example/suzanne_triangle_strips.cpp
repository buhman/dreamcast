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
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/pref.hpp"
#include "sh7091/store_queue_transfer.hpp"

#include "systembus/systembus.hpp"
#include "systembus/systembus_bits.hpp"

static inline void character(const char c)
{
  using sh7091::sh7091;
  using namespace sh7091;

  // set the transmit trigger to `1 byte`--this changes the behavior of TDFE
  sh7091.SCIF.SCFCR2 = scif::scfcr2::ttrg::trigger_on_1_bytes;

  // wait for transmit fifo to become partially empty
  while ((sh7091.SCIF.SCFSR2 & scif::scfsr2::tdfe::bit_mask) == 0);

  // unset tdfe bit
  sh7091.SCIF.SCFSR2 = (uint16_t)(~scif::scfsr2::tdfe::bit_mask);

  sh7091.SCIF.SCFTDR2 = c;
}

static void string(const char * s)
{
  while (*s != 0) {
    character(*s++);
  }
}

static void print_base16(uint32_t n, int len)
{
  char buf[len];
  char * bufi = &buf[len - 1];

  while (bufi >= buf) {
    uint32_t nib = n & 0xf;
    n = n >> 4;
    if (nib > 9) {
      nib += (97 - 10);
    } else {
      nib += (48 - 0);
    }

    *bufi = nib;
    bufi -= 1;
  }

  for (int i = 0; i < len; i++) {
    character(buf[i]);
  }
}

struct vec3 {
  float x;
  float y;
  float z;
};

#include "model/suzanne.h"
//#include "model/icosphere.h"

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
  polygon->vertex[0].base_color = 0x000000;

  polygon->vertex[1].x = 32.0f;
  polygon->vertex[1].y =  0.0f;
  polygon->vertex[1].z =  0.00001f;
  polygon->vertex[1].base_color = 0x000000;

  polygon->vertex[2].x = 32.0f;
  polygon->vertex[2].y = 32.0f;
  polygon->vertex[2].z =  0.00001f;
  polygon->vertex[2].base_color = 0x000000;
}

static inline uint32_t transfer_ta_global_end_of_list(uint32_t store_queue_ix)
{
  using namespace holly::ta;
  using namespace holly::ta::parameter;

  //
  // TA "end of list" global transfer
  //
  volatile global_parameter::end_of_list * end_of_list = (volatile global_parameter::end_of_list *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (global_parameter::end_of_list));

  end_of_list->parameter_control_word = parameter_control_word::para_type::end_of_list;

  // start store queue transfer of `end_of_list` to the TA
  pref(end_of_list);

  return store_queue_ix;
}

static inline uint32_t transfer_ta_global_polygon(uint32_t store_queue_ix)
{
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
                                  | parameter_control_word::col_type::floating_color;

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                    | isp_tsp_instruction_word::culling_mode::no_culling;
  // Note that it is not possible to use
  // ISP_TSP_INSTRUCTION_WORD::GOURAUD_SHADING in this isp_tsp_instruction_word,
  // because `gouraud` is one of the bits overwritten by the value in
  // parameter_control_word. See DCDBSysArc990907E.pdf page 200.

  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog;

  polygon->texture_control_word = 0;

  // start store queue transfer of `polygon` to the TA
  pref(polygon);

  return store_queue_ix;
}

#define abs(n) __builtin_abs(n)
#define cos(n) __builtin_cosf(n)
#define sin(n) __builtin_sinf(n)

static float theta = 0;

static inline vec3 vertex_rotate(vec3 v)
{
  // to make the models's appearance more interesting, rotate the vertex on two
  // axes

  float x0 = v.x;
  float y0 = v.y;
  float z0 = v.z;

  float x1 = x0 * cos(theta) - z0 * sin(theta);
  float y1 = y0;
  float z1 = x0 * sin(theta) + z0 * cos(theta);

  float x2 = x1;
  float y2 = y1 * cos(theta) - z1 * sin(theta);
  float z2 = y1 * sin(theta) + z1 * cos(theta);

  return (vec3){x2, y2, z2};
}

static inline vec3 vertex_perspective_divide(vec3 v)
{
  float w = 1.0f / (v.z + 2.0f);
  return (vec3){v.x * w, v.y * w, w};
}

static inline vec3 vertex_screen_space(vec3 v)
{
  return (vec3){
    v.x * 240.f + 320.f,
    v.y * 240.f + 240.f,
    v.z,
  };
}

static inline uint32_t transfer_ta_vertex_triangle(uint32_t store_queue_ix,
                                                   float x, float y, float z,
                                                   float r, float g, float b,
                                                   bool end_of_strip)
{
  using namespace holly::ta;
  using namespace holly::ta::parameter;

  //
  // TA polygon vertex transfer
  //

  volatile vertex_parameter::polygon_type_1 * vertex = (volatile vertex_parameter::polygon_type_1 *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (vertex_parameter::polygon_type_1)) * 1;

  vertex[0].parameter_control_word = parameter_control_word::para_type::vertex_parameter
                                   | (end_of_strip ? parameter_control_word::end_of_strip : 0);
  vertex[0].x = x;
  vertex[0].y = y;
  vertex[0].z = z;
  vertex[0].base_color_alpha = 1.0;
  vertex[0].base_color_r = r;
  vertex[0].base_color_g = g;
  vertex[0].base_color_b = b;

  pref(vertex);

  return store_queue_ix;
}

static const vec3 colors[] = {
  { 1.0, 0.0, 0.0 },
  { 1.0, 0.5454545454545454, 0.0 },
  { 0.9090909090909092, 1.0, 0.0 },
  { 0.36363636363636376, 1.0, 0.0 },
  { 0.0, 1.0, 0.18181818181818166 },
  { 0.0, 1.0, 0.7272727272727271 },
  { 0.0, 0.7272727272727275, 1.0 },
  { 0.0, 0.18181818181818166, 1.0 },
  { 0.3636363636363633, 0.0, 1.0 },
  { 0.9090909090909092, 0.0, 1.0 },
  { 1.0, 0.0, 0.5454545454545459 },
  { 1.0, 0.0, 0.0 },
  { 0.8500000000000001, 0.0, 0.0 },
  { 0.8500000000000001, 0.4636363636363636, 0.0 },
  { 0.7727272727272729, 0.8500000000000001, 0.0 },
  { 0.30909090909090925, 0.8500000000000001, 0.0 },
  { 0.0, 0.8500000000000001, 0.15454545454545443 },
  { 0.0, 0.8500000000000001, 0.618181818181818 },
  { 0.0, 0.6181818181818185, 0.8500000000000001 },
  { 0.0, 0.15454545454545443, 0.8500000000000001 },
  { 0.30909090909090886, 0.0, 0.8500000000000001 },
  { 0.7727272727272729, 0.0, 0.8500000000000001 },
  { 0.8500000000000001, 0.0, 0.463636363636364 },
  { 0.8500000000000001, 0.0, 0.0 },
  { 0.7, 0.0, 0.0 },
  { 0.7, 0.3818181818181818, 0.0 },
  { 0.6363636363636364, 0.7, 0.0 },
  { 0.25454545454545463, 0.7, 0.0 },
  { 0.0, 0.7, 0.12727272727272715 },
  { 0.0, 0.7, 0.5090909090909089 },
  { 0.0, 0.5090909090909093, 0.7 },
  { 0.0, 0.12727272727272715, 0.7 },
  { 0.2545454545454543, 0.0, 0.7 },
  { 0.6363636363636364, 0.0, 0.7 },
  { 0.7, 0.0, 0.38181818181818206 },
  { 0.7, 0.0, 0.0 },
  { 0.55, 0.0, 0.0 },
  { 0.55, 0.3, 0.0 },
  { 0.5000000000000001, 0.55, 0.0 },
  { 0.2000000000000001, 0.55, 0.0 },
  { 0.0, 0.55, 0.09999999999999992 },
  { 0.0, 0.55, 0.3999999999999999 },
  { 0.0, 0.4000000000000002, 0.55 },
  { 0.0, 0.09999999999999992, 0.55 },
  { 0.19999999999999984, 0.0, 0.55 },
  { 0.5000000000000001, 0.0, 0.55 },
  { 0.55, 0.0, 0.30000000000000027 },
  { 0.55, 0.0, 0.0 },
  { 0.39999999999999997, 0.0, 0.0 },
  { 0.39999999999999997, 0.21818181818181814, 0.0 },
  { 0.36363636363636365, 0.39999999999999997, 0.0 },
  { 0.1454545454545455, 0.39999999999999997, 0.0 },
  { 0.0, 0.39999999999999997, 0.07272727272727265 },
  { 0.0, 0.39999999999999997, 0.2909090909090908 },
  { 0.0, 0.290909090909091, 0.39999999999999997 },
  { 0.0, 0.07272727272727265, 0.39999999999999997 },
  { 0.1454545454545453, 0.0, 0.39999999999999997 },
  { 0.36363636363636365, 0.0, 0.39999999999999997 },
  { 0.39999999999999997, 0.0, 0.21818181818181834 },
  { 0.39999999999999997, 0.0, 0.0 },
  { 0.25, 0.0, 0.0 },
  { 0.25, 0.13636363636363635, 0.0 },
  { 0.2272727272727273, 0.25, 0.0 },
  { 0.09090909090909094, 0.25, 0.0 },
};
static const int strips_length = (sizeof (strips)) / (sizeof (strips[0]));

void transfer_ta_strips()
{
  {
    using namespace sh7091;
    using sh7091::sh7091;

    // set the store queue destination address to the TA Polygon Converter FIFO
    sh7091.CCN.QACR0 = sh7091::ccn::qacr0::address(ta_fifo_polygon_converter);
    sh7091.CCN.QACR1 = sh7091::ccn::qacr1::address(ta_fifo_polygon_converter);
  }

  uint32_t store_queue_ix = 0;

  store_queue_ix = transfer_ta_global_polygon(store_queue_ix);

  int color_ix = 0;

  for (int strip_ix = 0; strip_ix < strips_length; strip_ix++) {
    int vertex_ix = strips[strip_ix];

    vec3 vp = vertex_screen_space(
                vertex_perspective_divide(
                  vertex_rotate(vertices[abs(vertex_ix)])));

    const vec3& c = colors[color_ix];

    bool end_of_strip = vertex_ix < 0;

    store_queue_ix = transfer_ta_vertex_triangle(store_queue_ix,
                                                 vp.x, vp.y, vp.z,
                                                 c.x, c.y, c.z,
                                                 end_of_strip);

    if (end_of_strip) {
      color_ix = (color_ix + 1) % 64;
    }
  }

  store_queue_ix = transfer_ta_global_end_of_list(store_queue_ix);
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

  const int tile_y_num = 480 / 32;
  const int tile_x_num = 640 / 32;

  using namespace holly::core;

  region_array::list_block_size list_block_size = {
    .opaque = 32 * 4,
  };

  region_array::transfer(tile_x_num,
                         tile_y_num,
                         list_block_size,
                         region_array_start,
                         object_list_start);

  transfer_background_polygon(isp_tsp_parameter_start);

  //////////////////////////////////////////////////////////////////////////////
  // configure the TA
  //////////////////////////////////////////////////////////////////////////////

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
                      | ta_alloc_ctrl::o_opb::_32x4byte;

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

  holly.TA_NEXT_OPB_INIT = (object_list_start + 32 * 4 * tile_y_num * tile_x_num);

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

  // without waiting for rendering to actually complete, immediately display the
  // framebuffer.
  holly.FB_R_SOF1 = framebuffer_start;

  // draw 500 frames of cube rotation
  for (int i = 0; i < 5000; i++) {
    //////////////////////////////////////////////////////////////////////////////
    // transfer cube to texture memory via the TA polygon converter FIFO
    //////////////////////////////////////////////////////////////////////////////

    // TA_LIST_INIT needs to be written (every frame) prior to the first FIFO
    // write.
    holly.TA_LIST_INIT = ta_list_init::list_init;

    // dummy TA_LIST_INIT read; DCDBSysArc990907E.pdf in multiple places says this
    // step is required.
    (void)holly.TA_LIST_INIT;

    transfer_ta_strips();

    //////////////////////////////////////////////////////////////////////////////
    // wait for vertical synchronization (and the TA)
    //////////////////////////////////////////////////////////////////////////////

    while (!(spg_status::vsync(holly.SPG_STATUS)));
    while (spg_status::vsync(holly.SPG_STATUS));

    //////////////////////////////////////////////////////////////////////////////
    // start the actual rasterization
    //////////////////////////////////////////////////////////////////////////////

    // start the actual render--the rendering process begins by interpreting the
    // region array
    using systembus::systembus;
    using namespace systembus;
    systembus.ISTERR = 0xffffffff;

    holly.STARTRENDER = 1;

    while ((systembus.ISTNRM & istnrm::end_of_render_tsp) == 0) {
      if (systembus.ISTERR) {
        string("ISTERR: ");
        print_base16(systembus.ISTERR, 8);
        string("\n    ");
        return;
      }
    }
    systembus.ISTNRM = istnrm::end_of_render_tsp
                     | istnrm::end_of_render_isp
                     | istnrm::end_of_render_video;

    // increment theta for the cube rotation animation
    // (used by the `vertex_rotate` function)
    theta += 0.01f;
  }

  string("return\n    ");
  // return from main; this will effectively jump back to the serial loader
}
