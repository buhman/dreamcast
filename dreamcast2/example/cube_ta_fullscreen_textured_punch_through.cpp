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
#include "sh7091/store_queue_transfer.hpp"

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

static inline uint32_t transfer_ta_global_polygon(uint32_t store_queue_ix, uint32_t texture_address)
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
                                  | parameter_control_word::list_type::punch_through
                                  | parameter_control_word::col_type::packed_color
                                  | parameter_control_word::texture;

  polygon->isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                    | isp_tsp_instruction_word::culling_mode::no_culling;
  // Note that it is not possible to use
  // ISP_TSP_INSTRUCTION_WORD::GOURAUD_SHADING in this isp_tsp_instruction_word,
  // because `gouraud` is one of the bits overwritten by the value in
  // parameter_control_word. See DCDBSysArc990907E.pdf page 200.

  polygon->tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
                                | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                | tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::filter_mode::point_sampled
                                | tsp_instruction_word::texture_shading_instruction::decal
                                | tsp_instruction_word::texture_u_size::_128
                                | tsp_instruction_word::texture_v_size::_128;

  polygon->texture_control_word = texture_control_word::pixel_format::argb1555
                                | texture_control_word::scan_order::non_twiddled
                                | texture_control_word::texture_address(texture_address / 8);

  // start store queue transfer of `polygon` to the TA
  pref(polygon);

  return store_queue_ix;
}

static inline uint32_t transfer_ta_vertex_triangle(uint32_t store_queue_ix,
                                                   float ax, float ay, float az, float au, float av, uint32_t ac,
                                                   float bx, float by, float bz, float bu, float bv, uint32_t bc,
                                                   float cx, float cy, float cz, float cu, float cv, uint32_t cc)
{
  using namespace holly::ta;
  using namespace holly::ta::parameter;

  //
  // TA polygon vertex transfer
  //

  volatile vertex_parameter::polygon_type_3 * vertex = (volatile vertex_parameter::polygon_type_3 *)&store_queue[store_queue_ix];
  store_queue_ix += (sizeof (vertex_parameter::polygon_type_3)) * 3;

  // bottom left
  vertex[0].parameter_control_word = parameter_control_word::para_type::vertex_parameter;
  vertex[0].x = ax;
  vertex[0].y = ay;
  vertex[0].z = az;
  vertex[0].u = au;
  vertex[0].v = av;
  vertex[0].base_color = ac;
  vertex[0].offset_color = 0;

  // start store queue transfer of `vertex[0]` to the TA
  pref(&vertex[0]);

  // top center
  vertex[1].parameter_control_word = parameter_control_word::para_type::vertex_parameter;
  vertex[1].x = bx;
  vertex[1].y = by;
  vertex[1].z = bz;
  vertex[1].u = bu;
  vertex[1].v = bv;
  vertex[1].base_color = bc;
  vertex[1].offset_color = 0;

  // start store queue transfer of `vertex[1]` to the TA
  pref(&vertex[1]);

  // bottom right
  vertex[2].parameter_control_word = parameter_control_word::para_type::vertex_parameter
                                   | parameter_control_word::end_of_strip;
  vertex[2].x = cx;
  vertex[2].y = cy;
  vertex[2].z = cz;
  vertex[2].u = cu;
  vertex[2].v = cv;
  vertex[2].base_color = cc;
  vertex[2].offset_color = 0;

  // start store queue transfer of `params[2]` to the TA
  pref(&vertex[2]);

  return store_queue_ix;
}

/*
  These vertex and face definitions are a trivial transformation of the default
  Blender cube, as exported by the .obj exporter (with triangulation enabled).
 */
struct vec3 {
  float x;
  float y;
  float z;
};

struct vec2 {
  float u;
  float v;
};

static const vec3 cube_vertex_position[] = {
  {  1.0f,  1.0f, -1.0f },
  {  1.0f, -1.0f, -1.0f },
  {  1.0f,  1.0f,  1.0f },
  {  1.0f, -1.0f,  1.0f },
  { -1.0f,  1.0f, -1.0f },
  { -1.0f, -1.0f, -1.0f },
  { -1.0f,  1.0f,  1.0f },
  { -1.0f, -1.0f,  1.0f },
};

static const vec2 cube_vertex_texture[] = {
  { 1.0f, 0.0f },
  { 0.0f, 1.0f },
  { 0.0f, 0.0f },
  { 1.0f, 1.0f },
};

struct position_texture {
  int position;
  int texture;
};

struct face {
  position_texture a;
  position_texture b;
  position_texture c;
};

/*
  It is also possible to submit each cube face as a 4-vertex triangle strip, or
  submit the entire cube as a single triangle strip.

  Separate 3-vertex triangles are chosen to make this example more
  straightforward, but this is not the best approach if high performance is
  desired.
 */
static const face cube_faces[] = {
  {{4, 0}, {2, 1}, {0, 2}},
  {{2, 0}, {7, 1}, {3, 2}},
  {{6, 0}, {5, 1}, {7, 2}},
  {{1, 0}, {7, 1}, {5, 2}},
  {{0, 0}, {3, 1}, {1, 2}},
  {{4, 0}, {1, 1}, {5, 2}},
  {{4, 0}, {6, 3}, {2, 1}},
  {{2, 0}, {6, 3}, {7, 1}},
  {{6, 0}, {4, 3}, {5, 1}},
  {{1, 0}, {3, 3}, {7, 1}},
  {{0, 0}, {2, 3}, {3, 1}},
  {{4, 0}, {0, 3}, {1, 1}},
};
static const int cube_faces_length = (sizeof (cube_faces)) / (sizeof (cube_faces[0]));

#define cos(n) __builtin_cosf(n)
#define sin(n) __builtin_sinf(n)

static float theta = 0;

static inline vec3 vertex_rotate(vec3 v)
{
  // to make the cube's appearance more interesting, rotate the vertex on two
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
  float w = 1.0f / (v.z + 3.0f);
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

void transfer_ta_cube(uint32_t texture_address)
{
  {
    using namespace sh7091;
    using sh7091::sh7091;

    // set the store queue destination address to the TA Polygon Converter FIFO
    sh7091.CCN.QACR0 = sh7091::ccn::qacr0::address(ta_fifo_polygon_converter);
    sh7091.CCN.QACR1 = sh7091::ccn::qacr1::address(ta_fifo_polygon_converter);
  }

  uint32_t store_queue_ix = 0;

  store_queue_ix = transfer_ta_global_polygon(store_queue_ix, texture_address);

  for (int face_ix = 0; face_ix < cube_faces_length; face_ix++) {
    int ipa = cube_faces[face_ix].a.position;
    int ipb = cube_faces[face_ix].b.position;
    int ipc = cube_faces[face_ix].c.position;

    vec3 vpa = vertex_screen_space(
                 vertex_perspective_divide(
                   vertex_rotate(cube_vertex_position[ipa])));

    vec3 vpb = vertex_screen_space(
                 vertex_perspective_divide(
                   vertex_rotate(cube_vertex_position[ipb])));

    vec3 vpc = vertex_screen_space(
                 vertex_perspective_divide(
                   vertex_rotate(cube_vertex_position[ipc])));

    int ita = cube_faces[face_ix].a.texture;
    int itb = cube_faces[face_ix].b.texture;
    int itc = cube_faces[face_ix].c.texture;

    vec2 vta = cube_vertex_texture[ita];
    vec2 vtb = cube_vertex_texture[itb];
    vec2 vtc = cube_vertex_texture[itc];

    // vertex color is irrelevant in "decal" mode
    uint32_t va_color = 0;
    uint32_t vb_color = 0;
    uint32_t vc_color = 0;

    store_queue_ix = transfer_ta_vertex_triangle(store_queue_ix,
                                                 vpa.x, vpa.y, vpa.z, vta.u, vta.v, va_color,
                                                 vpb.x, vpb.y, vpb.z, vtb.u, vtb.v, vb_color,
                                                 vpc.x, vpc.y, vpc.z, vtc.u, vtc.v, vc_color);
  }

  store_queue_ix = transfer_ta_global_end_of_list(store_queue_ix);
}

const uint8_t texture[] __attribute__((aligned(4))) = {
  #embed "texture/ground_slam_128x128.rgb1555"
};

void transfer_texture(uint32_t texture_start)
{
  // use 4-byte transfers to texture memory, for slightly increased transfer
  // speed
  //
  // It would be even faster to use the SH4 store queue for this operation, or
  // SH4 DMA.

  sh7091::store_queue_transfer::copy((void *)&texture_memory64[texture_start], texture, (sizeof (texture)));
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

  // these addresses are in "64-bit" texture memory address space:
  uint32_t texture_start           = 0x700000;

  const int tile_y_num = 480 / 32;
  const int tile_x_num = 640 / 32;

  using namespace holly::core;

  region_array::list_block_size list_block_size = {
    .punch_through = 8 * 4,
  };

  region_array::transfer(tile_x_num,
                         tile_y_num,
                         list_block_size,
                         region_array_start,
                         object_list_start);

  transfer_background_polygon(isp_tsp_parameter_start);

  //////////////////////////////////////////////////////////////////////////////
  // transfer the texture image to texture ram
  //////////////////////////////////////////////////////////////////////////////

  transfer_texture(texture_start);

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
                      | ta_alloc_ctrl::pt_opb::_8x4byte;

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
  for (int i = 0; i < 500; i++) {
    //////////////////////////////////////////////////////////////////////////////
    // transfer cube to texture memory via the TA polygon converter FIFO
    //////////////////////////////////////////////////////////////////////////////

    // TA_LIST_INIT needs to be written (every frame) prior to the first FIFO
    // write.
    holly.TA_LIST_INIT = ta_list_init::list_init;

    // dummy TA_LIST_INIT read; DCDBSysArc990907E.pdf in multiple places says this
    // step is required.
    (void)holly.TA_LIST_INIT;

    transfer_ta_cube(texture_start);

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
    holly.STARTRENDER = 1;

    // increment theta for the cube rotation animation
    // (used by the `vertex_rotate` function)
    theta += 0.01f;
  }

  // return from main; this will effectively jump back to the serial loader
}
