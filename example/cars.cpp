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
#include "holly/texture_memory_alloc5.hpp"
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

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat2x2.hpp"
#include "math/mat3x3.hpp"
#include "math/mat4x4.hpp"
#include "math/geometry.hpp"
#include "math/transform.hpp"

#include "interrupt.hpp"

#include "assert.h"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#include "model/blender_export.h"
#include "model/cars/Wall_bricks_13_512px.data.h"
#include "model/cars/compact_classic/gulf_blue.data.h"
#include "model/cars/compact_classic/jupiter_grey.data.h"
#include "model/cars/garbage_truck/GarbageTruck.data.h"
#include "model/cars/scene.h"

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
                            | ta_alloc_ctrl::t_opb::no_list
                            | ta_alloc_ctrl::om_opb::no_list
                            | ta_alloc_ctrl::o_opb::_32x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
    .translucent = 0,
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

  if (istnrm & istnrm::end_of_transferring_opaque_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

    core_in_use = 1;
    core_start_render2(texture_memory_alloc.region_array.start,
                       texture_memory_alloc.isp_tsp_parameters.start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[framebuffer_ix].start,
                       framebuffer_width);

    ta_in_use = 0;
  }
}

void vbr600()
{
  uint32_t sr;
  asm volatile ("stc sr,%0" : "=r" (sr));
  sr |= sh::sr::imask(15);
  asm volatile ("ldc %0,sr" : : "r" (sr));
  //serial::string("imask\n");

  //check_pipeline();

  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) {
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(system.ISTERR);
    }

    pump_events(istnrm);

    sr &= ~sh::sr::imask(15);
    asm volatile ("ldc %0,sr" : : "r" (sr));

    return;
  }

  serial::string("vbr600\n");
  interrupt_exception();
}

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t control,
                           uint32_t tsp_instruction,
                           uint32_t texture_control_word,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_shading_instruction::decal
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction
                                      ;

  writer.append<ta_global_parameter::polygon_type_1>() =
    ta_global_parameter::polygon_type_1(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        a,
                                        r,
                                        g,
                                        b
                                        );
}

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               vec2 at,
                               vec2 bt,
                               vec2 ct,
                               vec2 dt,
                               float li)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        li, 0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        li, 0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        li, 0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        li, 0);
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

void transfer_mesh(ta_parameter_writer& writer, const mat4x4& trans, const object * object)
{
  const mesh * mesh = object->mesh;

  uint32_t control
    = para_control::list_type::opaque
    | obj_control::texture;

  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;

  if (mesh->materials_length != 0) {
    const mesh_material * material = &mesh->materials[0];

    tsp_instruction_word
      = tsp_instruction_word::texture_u_size::from_int(material->width)
      | tsp_instruction_word::texture_v_size::from_int(material->height);

    uint32_t texture_address = texture_memory_alloc.texture.start + material->offset;
    texture_control_word
      = texture_control_word::pixel_format::_565
      | texture_control_word::scan_order::twiddled
      | texture_control_word::texture_address(texture_address / 8);
  } else {
    tsp_instruction_word
      = tsp_instruction_word::texture_u_size::from_int(1024)
      | tsp_instruction_word::texture_v_size::from_int(512);

    uint32_t texture_address = texture_memory_alloc.texture.start + 1114112;
    texture_control_word
      = texture_control_word::pixel_format::_565
      | texture_control_word::scan_order::non_twiddled
      | texture_control_word::stride_select
      | texture_control_word::texture_address(texture_address / 8);
  }

  global_polygon_type_1(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word);

  mat4x4 trans1 = trans
    * translate(object->location)
    * rotate_quaternion(object->rotation)
    * scale(object->scale);

  vec3 position_cache[mesh->position_length];
  for (int i = 0; i < mesh->position_length; i++) {
    position_cache[i] = trans1 * mesh->position[i];
  }

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    vec3 ap = screen_transform(position_cache[p->a]);
    vec3 bp = screen_transform(position_cache[p->b]);
    vec3 cp = screen_transform(position_cache[p->c]);
    vec3 dp = screen_transform(position_cache[p->d]);

    vec2 at = mesh->uv_layers[0][p->uv_index + 0];
    vec2 bt = mesh->uv_layers[0][p->uv_index + 1];
    vec2 ct = mesh->uv_layers[0][p->uv_index + 2];
    vec2 dt = mesh->uv_layers[0][p->uv_index + 3];

    at.y = 1.0 - at.y;
    bt.y = 1.0 - bt.y;
    ct.y = 1.0 - ct.y;
    dt.y = 1.0 - dt.y;

    if (mesh->materials_length == 0) {
      at.x *= (640.f / 1024.f);
      at.y *= (480.f / 512.f);
      bt.x *= (640.f / 1024.f);
      bt.y *= (480.f / 512.f);
      ct.x *= (640.f / 1024.f);
      ct.y *= (480.f / 512.f);
      dt.x *= (640.f / 1024.f);
      dt.y *= (480.f / 512.f);
    }
    if (mesh == &mesh_Plane) {
      at.y *= 10;
      at.x *= 10;
      bt.y *= 10;
      bt.x *= 10;
      ct.y *= 10;
      ct.x *= 10;
      dt.y *= 10;
      dt.x *= 10;
    }

    float li = 1.0;

    render_quad(writer,
                ap, bp, cp, dp,
                at, bt, ct, dt,
                li);
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& trans)
{
  // opaque list
  {
    for (uint32_t i = 0; i < (sizeof (objects)) / (sizeof (objects[0])); i++) {
      transfer_mesh(writer, trans, &objects[i]);
    }

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

mat4x4 update_analog(mat4x4& screen_trans)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;
  int da = ft0::data_transfer::digital_button::da(data[0].digital_button) == 0;
  int ua = ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0;

  if (ra) {
    for (int i = 0; i < 5; i++)
      objects[i].location.x -= 0.05;
  }
  if (la) {
    for (int i = 0; i < 5; i++)
      objects[i].location.x += 0.05;
  }
  if (da) {
    for (int i = 0; i < 5; i++)
      objects[i].location.y += 0.05;
  }
  if (ua) {
    for (int i = 0; i < 5; i++)
      objects[i].location.y -= 0.05;
  }


  float y = -0.05f * x_;
  float x = 0.05f * y_;

  float z = -0.05f * r_ + 0.05f * l_;

  return translate((vec3){0, 0, z}) *
    screen_trans *
    rotate_x(x) *
    rotate_z(y);
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

void transfer_scene_textures()
{
  for (uint32_t i = 0; i < (sizeof (materials)) / (sizeof (materials[i])); i++) {
    uint32_t offset = texture_memory_alloc.texture.start + materials[i].offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    transfer_ta_fifo_texture_memory_32byte(dst, materials[i].start, materials[i].size);
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0;
  system.LMMODE1 = 0; // 64-bit

  transfer_scene_textures();
}

void dma_transfer(uint32_t source, uint32_t destination, uint32_t transfers)
{
  using namespace dmac;

  volatile uint32_t _dummy = sh7091.DMAC.CHCR1;
  (void)_dummy;

  sh7091.DMAC.CHCR1 = 0;

  sh7091.DMAC.SAR1 = source;
  sh7091.DMAC.DAR1 = destination;
  sh7091.DMAC.DMATCR1 = transfers & 0x00ff'ffff;

  sh7091.DMAC.CHCR1 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0100) /* auto request; external address space → external address space */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                  //| chcr::tm::cycle_steal_mode /* transmit mode */
                    | chcr::ts::_32_byte           /* transfer size */
                  //| chcr::ie::interrupt_request_generated
                    | chcr::de::channel_operation_enabled;
}

void ch2_dma_transfer(uint32_t source, uint32_t destination, uint32_t transfers)
{
  using namespace dmac;

  /*
  for (uint32_t i = 0; i < transfers; i++) {
    asm volatile ("ocbwb @%0"
		  :                          // output
		  : "r" (source + (32 * i)) // input
		  );
  }
  */

  // this dummy read appears to be required on real hardware.
  volatile uint32_t _dummy = sh7091.DMAC.CHCR2;
  (void)_dummy;

  system.ISTNRM = istnrm::end_of_dma_ch2_dma;

  /* start a new CH2-DMA transfer from "system memory" to "TA FIFO polygon converter" */
  sh7091.DMAC.CHCR2 = 0; /* disable DMA channel */
  sh7091.DMAC.SAR2 = reinterpret_cast<uint32_t>(source);  /* start address, must be aligned to a CHCHR__TS-sized (32-byte) boundary */
  sh7091.DMAC.DMATCR2 = dmatcr::transfer_count(transfers); /* transfer count, in CHCHR__TS-sized (32-byte) units */
  sh7091.DMAC.CHCR2 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0010) /* external request, single address mode;
					                   external address space → external device */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                    | chcr::ts::_32_byte         /* transfer size */
                    | chcr::de::channel_operation_enabled;

  system.C2DSTAT = c2dstat::texture_memory_start_address(destination); /* CH2-DMA destination address */
  system.C2DLEN  = c2dlen::transfer_length(transfers * 32);         /* CH2-DMA length (must be a multiple of 32) */
  system.C2DST   = 1;          /* CH2-DMA start (an 'external' request from SH7091's perspective) */

  // wait for ch2-dma completion
  while ((system.ISTNRM & istnrm::end_of_dma_ch2_dma) == 0);
  // reset ch2-dma interrupt status
  system.ISTNRM = istnrm::end_of_dma_ch2_dma;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 3];

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
                 | istnrm::end_of_transferring_opaque_list;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff202040);

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  video_output::set_mode_vga();

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 7,
    0, 0, 0, 1,
  };

  holly.TEXT_CONTROL = text_control::stride(20); // 640 pixels

  do_get_condition();
  int frame_ix = 0;
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    writer.offset = 0;

    screen_trans = update_analog(screen_trans);
    transfer_scene(writer, screen_trans);

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

    {
      uint32_t offset = texture_memory_alloc.framebuffer[next_frame_ix].start;
      void * in = (void *)&texture_memory32[offset / 4];

      uint32_t offset2 = texture_memory_alloc.texture.start + 1114112;
      void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset2 / 4]);
      dma_transfer((uint32_t)in, (uint32_t)dst, 640 * 480 * 2 / 32);
      while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0);
    }

    while (next_frame == 0);
    next_frame = 0;

    frame_ix += 1;
  }
}
