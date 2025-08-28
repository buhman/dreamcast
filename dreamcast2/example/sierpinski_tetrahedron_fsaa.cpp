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

// A blue
// B black
// C red
// D green
// A blue
// B black

struct vec3 {
  union {
    float x;
    float r;
  };
  union {
    float y;
    float g;
  };
  union {
    float z;
    float b;
  };
};

struct vertex {
  vec3 position;
  vec3 color;
};

constexpr float s = 2.5;

static const vertex tetrahedron_vertex[] = {
  {{ 0.500000 * s, -0.204124 * s,  0.288675 * s}, {0.0000, 0.0000, 1.0000}},
  {{ 0.000000 * s, -0.204124 * s, -0.577350 * s}, {0.0000, 0.0000, 0.0000}},
  {{-0.500000 * s, -0.204124 * s,  0.288675 * s}, {1.0000, 0.0000, 0.0000}},
  {{ 0.000000 * s,  0.612372 * s,  0.000000 * s}, {0.0000, 1.0000, 0.0000}},
};

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
                                  | parameter_control_word::col_type::floating_color
                                  | parameter_control_word::gouraud;

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                    | isp_tsp_instruction_word::culling_mode::cull_if_negative;
  // Note that it is not possible to use
  // ISP_TSP_INSTRUCTION_WORD::GOURAUD_SHADING in this isp_tsp_instruction_word,
  // because `gouraud` is one of the bits overwritten by the value in
  // parameter_control_word. See DCDBSysArc990907E.pdf page 200.

  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog;

  polygon->texture_control_word = 0;

  polygon->data_size_for_sort_dma = 0;
  polygon->next_address_for_sort_dma = 0;

  // start store queue transfer of `polygon` to the TA
  pref(polygon);

  return store_queue_ix;
}

static inline uint32_t transfer_ta_vertex_tetrahedron(uint32_t store_queue_ix,
                                                      const vec3& ap, const vec3& ac,
                                                      const vec3& bp, const vec3& bc,
                                                      const vec3& cp, const vec3& cc,
                                                      const vec3& dp, const vec3& dc)
{
  using namespace holly::ta;
  using namespace holly::ta::parameter;

  if (ap.z <= 0 || bp.z <= 0 || cp.z <= 0 || dp.z <= 0)
    return store_queue_ix;

  //
  // TA polygon vertex transfer
  //

  volatile vertex_parameter::polygon_type_1 * vertex = (volatile vertex_parameter::polygon_type_1 *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (vertex_parameter::polygon_type_1)) * 6;

#define transfer_vertex(n, p, c, pcw)                                   \
  vertex[n].parameter_control_word = pcw;                               \
  vertex[n].x = p.x;                                                    \
  vertex[n].y = p.y;                                                    \
  vertex[n].z = p.z;                                                    \
  vertex[n].base_color_r = c.r;                                         \
  vertex[n].base_color_g = c.g;                                         \
  vertex[n].base_color_b = c.b;                                         \
  pref(&vertex[n]);

  transfer_vertex(0, ap, ac, parameter_control_word::para_type::vertex_parameter);
  transfer_vertex(1, bp, bc, parameter_control_word::para_type::vertex_parameter);
  transfer_vertex(2, cp, cc, parameter_control_word::para_type::vertex_parameter);
  transfer_vertex(3, dp, dc, parameter_control_word::para_type::vertex_parameter);
  transfer_vertex(4, ap, ac, parameter_control_word::para_type::vertex_parameter);
  transfer_vertex(5, bp, bc, parameter_control_word::para_type::vertex_parameter | parameter_control_word::end_of_strip);

#undef transfer_vertex

  return store_queue_ix;
}

#define cos(n) __builtin_cosf(n)
#define sin(n) __builtin_sinf(n)

static float theta = 0;

static inline vec3 vertex_rotate(vec3 v, float cost, float sint)
{
  // to make the cube's appearance more interesting, rotate the vertex on two
  // axes

  float x0 = v.x;
  float y0 = v.y;
  float z0 = v.z;

  float x1 = x0 * cost - z0 * sint;
  float y1 = y0;
  float z1 = x0 * sint + z0 * cost;

  float x2 = x1;
  float y2 = y1 * cost - z1 * sint;
  float z2 = y1 * sint + z1 * cost;

  return (vec3){x2, y2, z2};
}

static inline vec3 vertex_perspective_divide(vec3 v)
{
  float w = 1.0f / (v.z + 1.f);
  return (vec3){v.x * w, v.y * w, w};
}

static inline vec3 vertex_screen_space(vec3 v)
{
  return (vec3){
    v.x * 480.f + 640.f,
    v.y * 240.f + 240.f,
    v.z,
  };
}

static uint32_t store_queue_ix = 0;
static float cost;
static float sint;

static inline void tetrahedron(vec3 a, vec3 b, vec3 c, vec3 d)
{
  vec3 ap = vertex_screen_space(
              vertex_perspective_divide(
                vertex_rotate(a, cost, sint)));

  vec3 bp = vertex_screen_space(
              vertex_perspective_divide(
                vertex_rotate(b, cost, sint)));

  vec3 cp = vertex_screen_space(
              vertex_perspective_divide(
                vertex_rotate(c, cost, sint)));

  vec3 dp = vertex_screen_space(
              vertex_perspective_divide(
                vertex_rotate(d, cost, sint)));

  const vec3& ac = tetrahedron_vertex[0].color;
  const vec3& bc = tetrahedron_vertex[1].color;
  const vec3& cc = tetrahedron_vertex[2].color;
  const vec3& dc = tetrahedron_vertex[3].color;

  store_queue_ix = transfer_ta_vertex_tetrahedron(store_queue_ix,
                                                  ap, ac,
                                                  bp, bc,
                                                  cp, cc,
                                                  dp, dc);
}

static inline vec3 midpoint(const vec3& a, const vec3& b)
{
  return {(a.x + b.x) * 0.5f,
	  (a.y + b.y) * 0.5f,
	  (a.z + b.z) * 0.5f};
}

static void subdivide(vec3 a, vec3 b, vec3 c, vec3 d,
                      int depth)
{
  if (depth == 0) {
    tetrahedron(a, b, c, d);
  } else {
    /*
        B
       / \
      A---C
     */

    vec3 ab = midpoint(a, b);
    vec3 ac = midpoint(a, c);
    vec3 ad = midpoint(a, d);
    vec3 bc = midpoint(b, c);
    vec3 bd = midpoint(b, d);
    vec3 cd = midpoint(c, d);

    /*
          b ----
         /  \   \
       ab    bc    \
       /      \      \
      a---ac---c--cd--d
     */


    subdivide( a, ab, ac, ad, depth - 1);
    subdivide(ab,  b, bc, bd, depth - 1);
    subdivide(ac, bc,  c, cd, depth - 1);
    subdivide(ad, bd, cd,  d, depth - 1);
  }
}

void transfer_ta_sierpinski_tetrahedron()
{
  {
    using namespace sh7091;
    using sh7091::sh7091;

    // set the store queue destination address to the TA Polygon Converter FIFO
    sh7091.CCN.QACR0 = sh7091::ccn::qacr0::address(ta_fifo_polygon_converter);
    sh7091.CCN.QACR1 = sh7091::ccn::qacr1::address(ta_fifo_polygon_converter);
  }

  store_queue_ix = 0;

  store_queue_ix = transfer_ta_global_polygon(store_queue_ix);

  cost = cos(theta);
  sint = sin(theta);

  subdivide(tetrahedron_vertex[0].position,
            tetrahedron_vertex[1].position,
            tetrahedron_vertex[2].position,
            tetrahedron_vertex[3].position,
            6);

  store_queue_ix = transfer_ta_global_end_of_list(store_queue_ix);
}

void main()
{
  /*
    a very simple memory map:

    the ordering within texture memory is not significant, and could be
    anything
  */
  uint32_t framebuffer_start[2]    = {0x000000, 0x12c000};
  uint32_t region_array_start      = 0x258000;
  uint32_t isp_tsp_parameter_start = 0x400000;
  uint32_t object_list_start       = 0x300000;

  const int tile_y_num = 480 / 32;
  const int tile_x_num = 1280 / 32;

  using namespace holly::core;

  region_array::list_block_size list_block_size = {
    .opaque = 8 * 4,
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
                      | ta_alloc_ctrl::o_opb::_8x4byte;

  // While building object lists, the TA contains an internal index (exposed as
  // the read-only TA_ITP_CURRENT) for the next address that new ISP/TSP will be
  // stored at. The initial value of this index is TA_ISP_BASE.

  // reserve space in ISP/TSP parameters for the background parameter
  using polygon = holly::core::parameter::isp_tsp_parameter<3>;
  uint32_t ta_isp_base_offset = (sizeof (polygon)) * 1;

  holly.TA_ISP_BASE = isp_tsp_parameter_start + ta_isp_base_offset;
  holly.TA_ISP_LIMIT = isp_tsp_parameter_start + 0x400000;

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
  holly.TA_OL_LIMIT = object_list_start + 0x200000 - 32;

  holly.TA_NEXT_OPB_INIT = (object_list_start + 8 * 4 * tile_y_num * tile_x_num);

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

  theta = 0;

  // draw 500 frames of cube rotation
  for (int i = 0; i < 1000; i++) {
    //////////////////////////////////////////////////////////////////////////////
    // transfer cube to texture memory via the TA polygon converter FIFO
    //////////////////////////////////////////////////////////////////////////////

    // TA_LIST_INIT needs to be written (every frame) prior to the first FIFO
    // write.
    holly.TA_LIST_INIT = ta_list_init::list_init;

    // dummy TA_LIST_INIT read; DCDBSysArc990907E.pdf in multiple places says this
    // step is required.
    volatile uint32_t init = holly.TA_LIST_INIT;
    (void)init;

    transfer_ta_sierpinski_tetrahedron();

    //////////////////////////////////////////////////////////////////////////////
    // start the actual rasterization
    //////////////////////////////////////////////////////////////////////////////

    using systembus::systembus;
    using namespace systembus;

    while ((systembus.ISTNRM & istnrm::end_of_transferring_opaque_list) == 0);
    systembus.ISTNRM = istnrm::end_of_transferring_opaque_list;

    holly.FB_W_SOF1 = framebuffer_start[i & 1];

    holly.SCALER_CTL = scaler_ctl::horizontal_scaling_enable
                     | scaler_ctl::vertical_scale_factor(0x0400);

    // start the actual render--the rendering process begins by interpreting the
    // region array
    holly.STARTRENDER = 1;

    while ((systembus.ISTNRM & istnrm::end_of_render_tsp) == 0);
    systembus.ISTNRM = istnrm::end_of_render_tsp
                     | istnrm::end_of_render_isp
                     | istnrm::end_of_render_video;


    // increment theta for the cube rotation animation
    // (used by the `vertex_rotate` function)
    theta += 0.001f;

    //////////////////////////////////////////////////////////////////////////////
    // wait for vertical synchronization
    //////////////////////////////////////////////////////////////////////////////

    while ((spg_status::vsync(holly.SPG_STATUS)));
    while (!(spg_status::vsync(holly.SPG_STATUS)));

    holly.FB_R_SOF1 = framebuffer_start[i & 1];
  }

  // return from main; this will effectively jump back to the serial loader
}
