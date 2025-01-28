#include <stdint.h>

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
#include "holly/texture_memory_alloc3.hpp"
#include "holly/video_output.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "memorymap.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "geometry/wiffle.hpp"

#include "sobel.hpp"

constexpr float half_degree = 0.01745329f / 2;

#define MODEL wiffle

vec3 rotate(const vec3& vertex, float theta)
{
  float x = vertex.x;
  float y = vertex.y;
  float z = vertex.z;
  float t;

  t  = y * cos(theta) - z * sin(theta);
  z  = y * sin(theta) + z * cos(theta);
  y  = t;

  float theta2 = 3.14 * sin(theta / 2);

  t  = x * cos(theta2) - z * sin(theta2);
  z  = x * sin(theta2) + z * cos(theta2);
  x  = t;

  return vec3(x, y, z);
}

void transform(const uint32_t face_ix,
               const float theta,
               const vec3 lights[3])
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
    //    | obj_control::texture
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::cull_if_positive;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::use_alpha;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
					isp_tsp_instruction_word,
					tsp_instruction_word,
					0, // texture_control_word
					0, // data_size_for_sort_dma
					0  // next_address_for_sort_dma
					);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  auto& face = MODEL::faces[face_ix];

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    // world transform
    uint32_t vertex_ix = face[i].vertex;
    auto& vertex = MODEL::vertices[vertex_ix];
    auto point = rotate(vertex, theta);

    // lighting transform
    uint32_t normal_ix = face[i].normal;
    auto& normal = MODEL::normals[normal_ix];
    auto n = rotate(normal, theta);

    /*
    vec4 color = {0.0, 0.0, 0.0, 1.0};

    // intensity calculation
    {
      auto l = lights[0] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
	float distance = magnitude(lights[0] - point);
	float attenuation = 1.0 / (1.0f
				   + 0.07f * distance
				   + 0.007f * (distance * distance));
	color.x += 5.0 * attenuation;
      }
    }

    {
      auto l = lights[1] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
	float distance = magnitude(lights[1] - point);
	float attenuation = 1.0 / (1.0f
				   + 0.07f * distance
				   + 0.007f * (distance * distance));
        color.y += 5.0 * attenuation;
      }
    }

    {
      auto l = lights[2] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
	float distance = magnitude(lights[2] - point);
	float attenuation = 1.0 / (1.0f
				   + 0.07f * distance
				   + 0.007f * (distance * distance));
        color.z += 9.0 * attenuation;
      }
    }
    */

    float x = point.x;
    float y = point.y;
    float z = point.z;

    x *= 1;
    y *= 1;
    z *= 1;

    // camera transform
    z += 90;

    // perspective
    x = x / z;
    y = y / z;

    // screen space transform
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;
    z = 1 / z;

    float scale_nx = ((n.x - -1) / (1 - -1)) * (1 - 0);
    float scale_ny = ((n.y - -1) / (1 - -1)) * (1 - 0);
    float scale_nz = ((n.z - -1) / (1 - -1)) * (1 - 0);
    float scale_z = ((point.z - -46) / (46 - -46)) * (1 - 0);

    bool end_of_strip = i == strip_length - 1;

    *reinterpret_cast<ta_vertex_parameter::polygon_type_1 *>(store_queue) =
      ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					  x, y, z,
					  scale_z, // alpha
					  scale_nx, // r
					  scale_ny, // g
                                          scale_nz  // b
					  );
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }
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

  for (uint32_t i = 0; i < transfers; i++) {
    asm volatile ("ocbwb @%0"
		  :                          // output
		  : "r" (source + (32 * i)) // input
		  );
  }

  // this dummy read appears to be required on real hardware.
  volatile uint32_t _dummy = sh7091.DMAC.CHCR2;
  (void)_dummy;

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

void dma_init()
{
  using namespace dmac;

  sh7091.DMAC.CHCR0 = 0;
  sh7091.DMAC.CHCR1 = 0;
  sh7091.DMAC.CHCR2 = 0;
  sh7091.DMAC.CHCR3 = 0;
  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */

}

static uint32_t inbuf[640 * 480] __attribute__((aligned(32)));
static float temp[640 * 480] __attribute__((aligned(32)));
static uint32_t outbuf[640 * 480] __attribute__((aligned(32)));

extern "C" int sobel_fipr(float * a, uint32_t * i);

void make_temp()
{
  for (int i = 0; i < 640 * 480; i++) {
    if ((i & 31) == 0) {
      asm volatile ("pref @%0"
                    :                          // output
                    : "r" ((uint32_t)&inbuf[i]) // input
                    );
    }
    uint32_t n = inbuf[i];
    uint32_t sum;
    sum = n & 0xff;
    n >>= 8;
    sum += n & 0xff;
    n >>= 8;
    sum += n & 0xff;
    n >>= 8;
    sum += n & 0xff;
    temp[i] = (float)(sum * 0.25);
  }
}

void main()
{
  dma_init();
  video_output::set_mode_vga();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::_16x4byte
			      | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::no_list;

  const int render_passes = 1;
  const struct opb_size opb_size[render_passes] = {
    {
      .opaque = 0,
      .opaque_modifier = 0,
      .translucent = 16 * 4,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  uint32_t frame_ix = 0;

  float theta = 0;
  vec3 lights[3] = {
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
  };

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc.region_array[0].start,
			 texture_memory_alloc.object_list[0].start);

  background_parameter2(texture_memory_alloc.background[0].start,
			0xffc0c0c0);

  while (1) {
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[0].start,
			       texture_memory_alloc.isp_tsp_parameters[0].end,
			       texture_memory_alloc.object_list[0].start,
			       texture_memory_alloc.object_list[0].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);

    float theta2 = 3.14 * 2 * sin(theta / 7);

    lights[0].x = cos(theta) * 20;
    lights[0].z = sin(theta) * 20;

    lights[1].x = cos(theta2 + half_degree * 180.f) * 20;
    lights[1].z = sin(theta2 + half_degree * 180.f) * 20;

    lights[2].x = cos(theta + half_degree * 360.f) * 20;
    lights[2].z = sin(theta + half_degree * 360.f) * 20;

    for (uint32_t i = 0; i < MODEL::num_faces; i++) {
      transform(i, theta, lights);
    }
    /*
    transform2(parameter, lights[0], {1.f, 0.f, 0.f, 1.f});
    transform2(parameter, lights[1], {0.f, 1.f, 0.f, 1.f});
    transform2(parameter, lights[2], {0.f, 0.f, 1.f, 1.f});
    */

    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    ta_wait_translucent_list();

    holly.FB_W_CTRL = fb_w_ctrl::fb_packmode::_8888_argb_32bit;
    uint32_t bytes_per_pixel = 4;
    core_start_render3(texture_memory_alloc.region_array[0].start,
                       texture_memory_alloc.isp_tsp_parameters[0].start,
                       texture_memory_alloc.background[0].start,
                       //texture_memory_alloc.framebuffer[0].start,
                       0x100'0000 | texture_memory_alloc.texture.start, // 64-bit area
                       framebuffer_width,
                       bytes_per_pixel);
    core_wait_end_of_render_video();

    uint32_t * in = (uint32_t *)&texture_memory64[texture_memory_alloc.texture.start / 4];
    //uint32_t * out = (uint32_t *)&texture_memory32[texture_memory_alloc.framebuffer[0].start / 4];

    //serial::string("ch1 dma start\n");
    dma_transfer((uint32_t)in, (uint32_t)inbuf, 640 * 480 * 4 / 32);

    for (uint32_t i = 0; i < (sizeof (640 * 480 * 4)) / 32; i++) {
      uint32_t address = (uint32_t)&inbuf[0];
      asm volatile ("ocbp @%0"
                    :                          // output
                    : "r" (address + (i * 32)) // input
                    );
    }

    while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0);
    //serial::string("ch1 dma end\n");

    //serial::string("temp start\n");
    make_temp();
    //serial::string("temp end\n");

    //serial::string("convolve start\n");

    //convolve(temp, outbuf);
    int a = sobel_fipr(temp, outbuf);
    //serial::integer<uint32_t>((uint32_t)temp);
    //serial::integer<uint32_t>(a);
    for (uint32_t i = 0; i < (640 * 480 * 4) / 32; i++) {
      uint32_t address = (uint32_t)&outbuf[0];
      asm volatile ("ocbwb @%0"
                    :                          // output
                    : "r" (address + (i * 32)) // input
                    );
    }

    //serial::string("convolve end\n");

    uint32_t framebuffer = 0x11000000 + texture_memory_alloc.framebuffer[0].start; // TA FIFO - Direct Texture Path
    system.LMMODE0 = 1;
    system.LMMODE1 = 1; // 32-bit
    //serial::string("ch2 dma start\n");
    ch2_dma_transfer((uint32_t)outbuf, framebuffer, (640 * 480 * 4) / 32);
    //serial::string("ch2 dma end\n");

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[0].start;
    while (spg_status::vsync(holly.SPG_STATUS));

    holly.FB_R_CTRL = fb_r_ctrl::vclk_div::pclk_vclk_1
                    | fb_r_ctrl::fb_depth::_0888_rgb_32bit
                    | fb_r_ctrl::fb_enable;

    holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                    | fb_r_size::fb_y_size(480 - 3)
                    | fb_r_size::fb_x_size((640 * 32) / 32 - 1);

    theta += half_degree;
  }
}
