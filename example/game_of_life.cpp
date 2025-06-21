#include <bit>

#include "holly/background.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/texture_memory_alloc7.hpp"
#include "holly/video_output.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "memorymap.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"

#include "math/float_types.hpp"
#include "math/transform.hpp"

#include "interrupt.hpp"
#include "assert.h"

#include "texture/game_of_life/dead.data.h"
#include "texture/game_of_life/live1.data.h"
#include "texture/game_of_life/live2.data.h"
#include "texture/game_of_life/live3.data.h"
#include "texture/game_of_life/live4.data.h"

const int max_knot_segments = 32;
const int max_knot_rings = 256;
int knot_segments = 32;
int knot_rings = 256;
//int knot_rings = 32;
vec3 _knot_center[max_knot_rings];
vec3 _knot_ring[max_knot_rings][max_knot_segments];
//vec3 t_knot_center[max_knot_rings];
vec3 t_knot_ring[max_knot_rings][max_knot_segments];

static inline vec3 knot(const float t)
{
  float x = sin(t) + 2 * sin(2 * t);
  float y = cos(t) - 2 * cos(2 * t);
  float z = -sin(3 * t);
  return {x, y, z};
}

static inline vec3 rodrigues_rotation(const vec3 v, const vec3 k, const float t)
{
  return v * cos(t) + cross(k, v) * sin(t) + k * dot(k, v) * (1 - cos(t));
}

static inline void radial_segments(const vec3 a,
                                   const vec3 n0,
                                   const vec3 n,
                                   const int segments,
                                   vec3 * radial_surface)
{
  for (int i = 0; i < segments; i++) {
    float t = ((float)i / (float)segments) * 2.f * pi;
    vec3 rn = rodrigues_rotation(n, n0, t);
    rn = normalize(rn);
    radial_surface[i] = a + rn * 0.6f;
  }
}


static inline vec3 knot_center(const float i, const float rings)
{
  float t = (i / rings) * 2.f * pi;
  vec3 center = knot(t);
  return center;
}

void knot_edges(const int rings, const int segments)
{
  for (int i = 0; i < rings; i++) {
    vec3 center = knot_center(i, rings);
    _knot_center[i] = center;
  }

  for (int i = 0; i < rings; i++) {
    int ip = (i + 1) & (rings - 1);
    int im = (i - 1) & (rings - 1);

    const vec3& a = _knot_center[i];
    const vec3& b = _knot_center[ip];
    const vec3& c = _knot_center[im];

    vec3 n0 = ((b - a) + (a - c)) * 0.5f;
    n0 = normalize(n0);
    vec3 n = cross(n0, -a);
    n = normalize(n);

    radial_segments(a, n0, n, segments, &_knot_ring[i][0]);
  }
}

struct grid {
  int width;
  int height;
  int generation;
  int * data[2];
};

static inline int grid_get(grid const * const grid, int x, int y)
{
  x = x & (grid->width - 1);
  y = y & (grid->height - 1);
  int gen = grid->generation & 1;

  return grid->data[gen][y * grid->width + x];
}

static inline void grid_put_p(grid * const grid, int x, int y, int value)
{
  x = x & (grid->width - 1);
  y = y & (grid->height - 1);
  int gen = grid->generation & 1;

  grid->data[gen][y * grid->width + x] = value;
}

static inline void grid_put(grid * const grid, int x, int y, int value)
{
  x = x & (grid->width - 1);
  y = y & (grid->height - 1);
  int gen = !(grid->generation & 1);

  grid->data[gen][y * grid->width + x] = value;
}

static inline int count_neighbors(grid const * const grid, int x, int y)
{
  int count = 0;
  count += grid_get(grid, x - 1, y - 1) > 0;
  count += grid_get(grid, x - 0, y - 1) > 0;
  count += grid_get(grid, x + 1, y - 1) > 0;
  count += grid_get(grid, x - 1, y - 0) > 0;
  //count += grid_get(grid, x - 0, y - 0) > 0;
  count += grid_get(grid, x + 1, y - 0) > 0;
  count += grid_get(grid, x - 1, y + 1) > 0;
  count += grid_get(grid, x - 0, y + 1) > 0;
  count += grid_get(grid, x + 1, y + 1) > 0;
  return count;
}

static inline void apply_rule(grid * grid, int x, int y)
{
  int live = grid_get(grid, x, y);
  int count = count_neighbors(grid, x, y);
  if (live < 0) {
    // do nothing
  } else if (live > 0) {
    if (count < 2)
      live = 0;
    else if (count > 3)
      live = 0;
    else if (live < 4)
      live += 1;
  } else { // live == 0
    if (count == 3)
      live = 1;
  }
  grid_put(grid, x, y, live);
}

void grid_generation(grid * grid)
{
  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      apply_rule(grid, x, y);
    }
  }
  grid->generation += 1;
}

void seed_grid(grid * grid, int xo, int yo)
{
  static const uint8_t seed[] = {
    0, 1, 0,
    1, 1, 0,
    0, 1, 1,
  };

  const int seed_width = 3;
  const int seed_height = 3;

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      if (y < seed_height && x < seed_width) {
        grid_put(grid, xo + x, yo + y, seed[y * seed_width + x]);
      }
    }
  }
}

// cell points to next cell
struct cell {
  int x;
  int y;
};

cell snake_unpack_cell(int a)
{
  assert(a < 0);
  int x = ((uint32_t)a >> 0) & 0xff;
  int y = ((uint32_t)a >> 8) & 0xff;
  return {x, y};
}

int snake_pack_cell(cell c)
{
  uint32_t v = (1 << 31)
    | (((uint32_t)c.x & 0xff) << 0)
    | (((uint32_t)c.y & 0xff) << 8);
  return v;
}

enum direction : int {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

struct snake {
  cell head;
  cell tail;
  enum direction direction;
};

static inline cell move(cell p, int d)
{
  switch (d) {
  case UP:    return {p.x, p.y - 1};
  case DOWN:  return {p.x, p.y + 1};
  case LEFT:  return {p.x - 1, p.y};
  case RIGHT: return {p.x + 1, p.y};
  }
  assert(false);
}

void snake_move(grid * grid, snake * snake, bool force_grow)
{
  cell head = move(snake->head, snake->direction);

  int live = grid_get(grid, head.x, head.y);

  grid_put_p(grid, head.x, head.y, -1);

  grid_put_p(grid, snake->head.x, snake->head.y,
             snake_pack_cell(head));

  snake->head.x = head.x;
  snake->head.y = head.y;

  int grow = live > 0 || force_grow;

  if (!grow) {
    cell tail = snake_unpack_cell(grid_get(grid, snake->tail.x, snake->tail.y));

    grid_put_p(grid, snake->tail.x, snake->tail.y, 0);

    snake->tail.x = tail.x;
    snake->tail.y = tail.y;
  }
}

void snake_init(grid * grid, snake * snake, int x, int y)
{
  snake->head = {x - 1, y};
  snake->tail = {x - 1, y};
  snake->direction = RIGHT;

  snake_move(grid, snake, true);
}

static ft0::data_transfer::data_format data[4];

uint8_t send_buf[1024] __attribute__((aligned(32)));
uint8_t recv_buf[1024] __attribute__((aligned(32)));

void do_get_condition()
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  for (int port = 0; port < 4; port++) {
    auto& data_fields = host_command[port].bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::controller);
  }
  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((std::byteswap(data_fields.function_type) & function_type::controller) == 0) {
      return;
    }

    data[port].digital_button = data_fields.data.digital_button;
    for (int i = 0; i < 6; i++) {
      data[port].analog_coordinate_axis[i]
        = data_fields.data.analog_coordinate_axis[i];
    }
  }
}

void vbr100()
{
  serial::string("vbr100\n");
  interrupt_exception();
}

void vbr400()
{
  serial::string("vbr400\n");
  interrupt_exception();
}

const int framebuffer_width = 640;
const int framebuffer_height = 480;
const int tile_width = framebuffer_width / 32;
const int tile_height = framebuffer_height / 32;

constexpr uint32_t ta_alloc = 0
                            | ta_alloc_ctrl::pt_opb::no_list
  | ta_alloc_ctrl::tm_opb::no_list
  | ta_alloc_ctrl::t_opb::_32x4byte
  | ta_alloc_ctrl::om_opb::no_list
  | ta_alloc_ctrl::o_opb::no_list
  ;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 0,
    .opaque_modifier = 0,
    .translucent = 32 * 4,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};

static volatile int ta_in_use = 0;
static volatile int core_in_use = 0;
static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static inline void pump_events(uint32_t istnrm)
{
  if (istnrm & istnrm::v_blank_in) {
    system.ISTNRM = istnrm::v_blank_in;

    next_frame = 1;
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[next_frame_ix].start;
  }

  if (istnrm & istnrm::end_of_render_tsp) {
    system.ISTNRM = istnrm::end_of_render_tsp
                  | istnrm::end_of_render_isp
                  | istnrm::end_of_render_video;

    next_frame_ix = framebuffer_ix;
    framebuffer_ix += 1;
    if (framebuffer_ix >= 3) framebuffer_ix = 0;

    core_in_use = 0;
  }

  if (istnrm & istnrm::end_of_transferring_translucent_list) {
    system.ISTNRM = istnrm::end_of_transferring_translucent_list;

    core_in_use = 1;

    holly.FB_W_SOF1 = texture_memory_alloc.framebuffer[framebuffer_ix].start;
    holly.STARTRENDER = 1;

    ta_in_use = 0;
  }
}

void vbr600()
{
  uint32_t sr;
  asm volatile ("stc sr,%0" : "=r" (sr));
  sr |= sh::sr::imask(15);
  asm volatile ("ldc %0,sr" : : "r" (sr));

  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) {
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(isterr);
      if (isterr & 1) {
        system.ISTERR = 1;
      }
    }

    pump_events(istnrm);

    sr &= ~sh::sr::imask(15);
    asm volatile ("ldc %0,sr" : : "r" (sr));

    return;
  }

  serial::string("vbr600\n");
  interrupt_exception();
}

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater_or_equal
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::texture_shading_instruction::decal
                                ;

  uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0,
                                        0);
}

void global_polygon_type_0_packed(ta_parameter_writer& writer,
                                  uint32_t para_control_obj_control,
                                  int ix
                                  )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater_or_equal
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::src_alpha_instr::src_alpha
                                | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                | tsp_instruction_word::texture_shading_instruction::decal
                                | tsp_instruction_word::texture_u_size::from_int(32)
                                | tsp_instruction_word::texture_v_size::from_int(32)
                                | tsp_instruction_word::filter_mode::bilinear_filter
                                ;

  uint32_t texture_address = texture_memory_alloc.texture.start + (32 * 32 * 2) * ix;
  uint32_t texture_control_word = texture_control_word::pixel_format::_1555
                                | texture_control_word::scan_order::twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0,
                                        0);
}

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               vec3 ac,
                               vec3 bc,
                               vec3 cc,
                               vec3 dc)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        1, ac.r, ac.g, ac.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        1, bc.r, bc.g, bc.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        1, dc.r, dc.g, dc.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        1, cc.r, cc.g, cc.b);
}

static inline void render_quad2(ta_parameter_writer& writer,
                                vec3 ap,
                                vec3 bp,
                                vec3 cp,
                                vec3 dp)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        0, 0,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        0, 1,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        1, 0,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        1, 1,
                                        0, 0);
}

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480 / 2.0;

  return {
    v.x / (1.f * v.z) * dim + 640 / 2.0f,
    v.y / (1.f * v.z) * dim + 480 / 2.0f,
    1 / v.z,
  };
}

static int last_value = -1;

static inline void transfer_knot_face(ta_parameter_writer& writer, const grid * grid, int r0, int r1, int s0, int s1)
{
  // x, y
  int value = grid_get(grid, r0, s0);
  if (value > 0)
    value = 0;
  if (value < 0)
    value = 1;

  if (last_value != value) {
    global_polygon_type_0_packed(writer,
                                 para_control::list_type::translucent,
                                 value);
    last_value = value;
  }

  render_quad2(writer,
               t_knot_ring[r0][s0],
               t_knot_ring[r0][s1],
               t_knot_ring[r1][s1],
               t_knot_ring[r1][s0]);
}

static inline void transfer_knot_inner(ta_parameter_writer& writer, const grid * grid, int r0, int r1)
{
  for (int s0 = 0; s0 < knot_segments - 1; s0++) {
    int s1 = s0 + 1;
    transfer_knot_face(writer, grid, r0, r1, s0, s1);
  }
  transfer_knot_face(writer, grid, r0, r1, knot_segments - 1, 0);
}

void transfer_knot(ta_parameter_writer& writer, mat4x4& trans, const grid * grid)
{
  for (int i = 0; i < knot_rings; i++) {
    //t_knot_center[i] = screen_transform(trans * _knot_center[i]);
    for (int j = 0; j < knot_segments; j++) {
      t_knot_ring[i][j] = screen_transform(trans * _knot_ring[i][j]);
    }
  }

  for (int r0 = 0; r0 < knot_rings - 1; r0++) {
    int r1 = r0 + 1;
    transfer_knot_inner(writer, grid, r0, r1);
  }
  transfer_knot_inner(writer, grid, knot_rings - 1, 0);
}

void transfer_grid(ta_parameter_writer& writer, const grid * grid)
{
  float dim = 10;

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      int value = grid_get(grid, x, y);
      if (value == 0)
        continue;

      float fx = x;
      float fx1 = x + 1;
      float fy = y;
      float fy1 = y + 1;

      vec3 a = {dim * fx , dim * fy , 0.001f};
      vec3 b = {dim * fx1, dim * fy , 0.001f};
      vec3 c = {dim * fx1, dim * fy1, 0.001f};
      vec3 d = {dim * fx , dim * fy1, 0.001f};

      mat4x4 r = translate((vec3){20, 20, 0});
      vec3 color;
      if (value > 0)
        color = {1, 1, 1};
      else {
        color = {0, 0, 1};
      }
      render_quad(writer,
                  r * a,
                  r * b,
                  r * c,
                  r * d,
                  color,
                  color,
                  color,
                  color);
    }
  }
}

void transfer_scene(ta_parameter_writer& writer, grid * grid, mat4x4& trans)
{
  global_polygon_type_0(writer,
                        para_control::list_type::translucent);
  transfer_grid(writer, grid);

  last_value = -1;
  transfer_knot(writer, trans, grid);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, const void * src, int length)
{
  assert((((int)dst) & 31) == 0);
  assert((((int)length) & 31) == 0);

  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffe0) / 4];
  const uint32_t * src32 = reinterpret_cast<const uint32_t *>(src);

  length = (length + 31) & ~31; // round up to nearest multiple of 32
  while (length > 0) {
    base[0] = src32[0];
    base[1] = src32[1];
    base[2] = src32[2];
    base[3] = src32[3];
    base[4] = src32[4];
    base[5] = src32[5];
    base[6] = src32[6];
    base[7] = src32[7];
    asm volatile ("pref @%0"
                  :                // output
                  : "r" (&base[0]) // input
                  : "memory");
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_texture()
{
  const void * start[5] = {
    (void *)&_binary_texture_game_of_life_dead_data_start,
    (void *)&_binary_texture_game_of_life_live1_data_start,
    (void *)&_binary_texture_game_of_life_live2_data_start,
    (void *)&_binary_texture_game_of_life_live3_data_start,
    (void *)&_binary_texture_game_of_life_live4_data_start,
  };

  for (uint32_t i = 0; i < (sizeof (start)) / (sizeof (start[0])); i++) {
    uint32_t offset = texture_memory_alloc.texture.start + (32 * 32 * 2) * i;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    const void * src = start[i];
    uint32_t size = 32 * 32 * 2;
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  transfer_texture();
}

static inline mat4x4 update_analog(const mat4x4& screen_trans)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;
  float x = 0.05f * -x_;
  float y = 0.05f * y_;

  float z = 1.0 + (-0.01f * r_ + 0.01f * l_);

  mat4x4 s = scale((vec3){z, z, z});
  mat4x4 ry = rotate_x(x);
  mat4x4 rz = rotate_z(y);

  return screen_trans * s * ry * rz;
}

static inline void update_digital(snake * snake)
{
  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;
  int da = ft0::data_transfer::digital_button::da(data[0].digital_button) == 0;
  int ua = ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0;

  if (ra) {
    snake->direction = RIGHT;
  }
  if (la) {
    snake->direction = LEFT;
  }
  if (ua) {
    snake->direction = UP;
  }
  if (da) {
    snake->direction = DOWN;
  }
}

static inline vec3 lerp(vec3 a, vec3 b, float t)
{
  return a + (b - a) * t;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 2];

int main()
{
  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  serial::init(0);

  interrupt_init();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  transfer_textures();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_translucent_list;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff000000);

  video_output::set_mode_vga();

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  {
    uint32_t region_array_start = texture_memory_alloc.region_array.start;
    uint32_t isp_tsp_parameters_start = texture_memory_alloc.isp_tsp_parameters.start;
    uint32_t background_start = texture_memory_alloc.background[0].start;

    holly.REGION_BASE = region_array_start;
    holly.PARAM_BASE = isp_tsp_parameters_start;

    uint32_t background_offset = background_start - isp_tsp_parameters_start;
    holly.ISP_BACKGND_T = isp_backgnd_t::tag_address(background_offset / 4)
                        | isp_backgnd_t::tag_offset(0)
                        | isp_backgnd_t::skip(1);
    holly.ISP_BACKGND_D = _i(1.f/100000.f);

    holly.FB_W_CTRL = fb_w_ctrl::fb_dither
                    | fb_w_ctrl::fb_packmode::_565_rgb_16bit;
    uint32_t bytes_per_pixel = 2;
    holly.FB_W_LINESTRIDE = (framebuffer_width * bytes_per_pixel) / 8;
  }

  const int max_width = max_knot_rings;
  const int max_height = max_knot_segments;
  static int grid_a[max_width * max_height] = {};
  static int grid_b[max_width * max_height] = {};
  grid grid = {
    .width = knot_rings,
    .height = knot_segments,
    .generation = 1,
    .data = {grid_a, grid_b},
  };
  for (int i = 0; i < 8; i++) {
    seed_grid(&grid, 32 * i, 0);
  }
  snake snake;
  grid.generation = 0;
  snake_init(&grid, &snake, 5, 5);

  int tick = 0;

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1,
  };

  knot_edges(knot_rings, knot_segments);

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    //screen_trans = update_analog(screen_trans);
    update_digital(&snake);

    constexpr int ticks_per_animation_frame = 16;

    if ((tick & (ticks_per_animation_frame - 1)) == 0) {
      grid_generation(&grid);
      snake_move(&grid, &snake, false);
    }

    /*
    constexpr float tick_div = 1.0f / (float)ticks_per_animation_frame;
    int anim_tick = -tick;
    int anim_frame = anim_tick / ticks_per_animation_frame;
    float t = (anim_tick - (anim_frame * ticks_per_animation_frame)) * tick_div;
    int eye0 = (anim_frame + 0) & (knot_rings - 1);
    int eye1 = (anim_frame + 1) & (knot_rings - 1);
    int center0 = (anim_frame + 1) & (knot_rings - 1);
    int center1 = (anim_frame + 2) & (knot_rings - 1);

    vec3 eye = lerp(_knot_center[eye0], _knot_center[eye1], t);
    vec3 center = lerp(_knot_center[center0], _knot_center[center1], t);
    vec3 up = lerp(_knot_ring[eye0][0], _knot_ring[eye1][0], t);
    */

    int ex = (snake.head.x - 10) & (grid.width - 1);
    int cx = (snake.head.x + 0) & (grid.width - 1);
    int y = (snake.head.y) & (grid.height - 1);

    vec3 up     = -_knot_ring[cx][snake.head.y];
    vec3 eye    = _knot_center[ex];
    vec3 center = -_knot_center[cx];

    screen_trans = look_at(eye, center, up);

    writer.offset = 0;
    transfer_scene(writer, &grid, screen_trans);

    tick += 1;
    if ((tick & 3) == 0) {
      //grid_generation(&grid);
    }

    while (ta_in_use);
    while (core_in_use);
    ta_in_use = 1;
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters.start,
                               texture_memory_alloc.isp_tsp_parameters.end,
                               texture_memory_alloc.object_list.start,
                               texture_memory_alloc.object_list.end,
                               opb_size[0].total(),
                               ta_alloc,
                               tile_width,
                               tile_height);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);



    while (next_frame == 0);
    next_frame = 0;
  }
}
