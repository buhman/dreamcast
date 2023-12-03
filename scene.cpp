#include <cstdint>

#include "holly/region_array.h"
#include "holly/background.h"
#include "holly/ta_parameter.h"
#include "holly/core_bits.h"
#include "holly.h"
#include "ta.h"
#include "sh7091.h"

#include "memorymap.h"
#include "storequeue.h"
#include "systembus.h"
#include "holly/float_uint32.h"

struct texture_memory_alloc {
  uint32_t isp_tsp_parameters[0x00100000 / 4]; // TA_ISP_BASE / PARAM_BASE (the actual objects)
  uint32_t        object_list[0x00100000 / 4]; // TA_OL_BASE (contains object pointer blocks)
  uint32_t              _res0[      0x20 / 4]; // (the TA may clobber 4 bytes starting at TA_OL_LIMIT)
  uint32_t       region_array[0x00002000 / 4]; // REGION_BASE
  uint32_t         background[0x00000020 / 4]; // ISP_BACKGND_T
  uint32_t     framebuffer[2][0x00096000 / 4]; // FB_R_SOF1 / FB_W_SOF1
};

/*
    0,-.5
      |
            ---
  -0.5,0.5    |  0.5,0.5
 */

float scene_triangle[3][3] = {
  { 0.f,  -0.5f,  1/10.f},
  { 0.5f,  0.5f,  1/10.f},
  { -0.5f,  0.5f,  1/10.f},
};

void scene_holly_init()
{
  holly.ISP_FEED_CFG   = ISP_FEED_CFG__TRANSLUCENCY_CACHE_SIZE(0x200);

  holly.FPU_SHAD_SCALE = FPU_SHAD_SCALE__SCALE_FACTOR(1);
  holly.FPU_CULL_VAL   = _i(1.f);
  holly.FPU_PERP_VAL   = _i(0.f);
  holly.SPAN_SORT_CFG  = SPAN_SORT_CFG__SPAN_SORT_ENABLE
                       | SPAN_SORT_CFG__OFFSET_SORT_ENABLE;

  holly.FOG_COL_RAM    = FOG_COL_RAM__RED(127)
		       | FOG_COL_RAM__GREEN(127)
		       | FOG_COL_RAM__BLUE(127);

  holly.FOG_COL_VERT   = FOG_COL_VERT__RED(127)
		       | FOG_COL_VERT__GREEN(127)
		       | FOG_COL_VERT__BLUE(127);

  holly.FOG_CLAMP_MIN  = FOG_CLAMP_MIN__ALPHA(0)
                       | FOG_CLAMP_MIN__RED(0)
                       | FOG_CLAMP_MIN__GREEN(0)
                       | FOG_CLAMP_MIN__BLUE(0);

  holly.FOG_CLAMP_MAX  = FOG_CLAMP_MAX__ALPHA(255)
                       | FOG_CLAMP_MAX__RED(255)
                       | FOG_CLAMP_MAX__GREEN(255)
                       | FOG_CLAMP_MAX__BLUE(255);

  holly.HALF_OFFSET    = HALF_OFFSET__TSP_TEXEL_SAMPLE_POSITION_CENTER
		       | HALF_OFFSET__TSP_PIXEL_SAMPLE_POSITION_CENTER
		       | HALF_OFFSET__FPU_PIXEL_SAMPLE_POSITION_CENTER;

  holly.FPU_PARAM_CFG  = FPU_PARAM_CFG__REGION_HEADER_TYPE__2
		       | FPU_PARAM_CFG__TSP_PARAMETER_BURST_TRIGGER(31)
		       | FPU_PARAM_CFG__ISP_PARAMETER_BURST_TRIGGER(31)
		       | FPU_PARAM_CFG__POINTER_BURST_SIZE(7) // must be less than OPB size
		       | FPU_PARAM_CFG__POINTER_FIRST_BURST_SIZE(7); // half of pointer burst size(?)
}

void scene_ta_init()
{
  holly.SOFTRESET = SOFTRESET__TA_SOFT_RESET;
  holly.SOFTRESET = 0;

  holly.TA_GLOB_TILE_CLIP = TA_GLOB_TILE_CLIP__TILE_Y_NUM((480 / 32) - 1)
                          | TA_GLOB_TILE_CLIP__TILE_X_NUM((640 / 32) - 1);

  holly.TA_ALLOC_CTRL = TA_ALLOC_CTRL__OPB_MODE_INCREASING
		      | TA_ALLOC_CTRL__PT_OPB__NONE
		      | TA_ALLOC_CTRL__TM_OPB__NONE
		      | TA_ALLOC_CTRL__T_OPB__NONE
		      | TA_ALLOC_CTRL__OM_OPB__NONE
		      | TA_ALLOC_CTRL__O_OPB__16;

  holly.TA_ISP_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));
  holly.TA_ISP_LIMIT = (offsetof (struct texture_memory_alloc, object_list)); // the end of isp_tsp_parameters
  holly.TA_OL_BASE = (offsetof (struct texture_memory_alloc, object_list));
  holly.TA_OL_LIMIT = (offsetof (struct texture_memory_alloc, _res0)); // the end of the object_list
  holly.TA_NEXT_OPB_INIT = (offsetof (struct texture_memory_alloc, object_list));
  //holly.TA_NEXT_OPB_INIT = (offsetof (struct texture_memory_alloc, object_list))
  //                       + (640 / 32) * (320 / 32) * 16 * 4;

  holly.TA_LIST_INIT = TA_LIST_INIT__LIST_INIT;

  volatile uint32_t _dummy_read = holly.TA_LIST_INIT;
  (void)_dummy_read;
}

void scene_wait_opaque_list()
{
  while ((system.ISTNRM & SB_ISTNRM__TAEOINT) == 0);

  system.ISTNRM = SB_ISTNRM__TAEOINT;
}

static float theta = 0;
constexpr float degree = 0.01745329f;

void scene_geometry_transfer()
{
  /*
  triangle(store_queue);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  for (int i = 0; i < 3; i++) {
    bool end_of_strip = i == 2;

    vertex(store_queue,
           scene_triangle[i][0], // x
           scene_triangle[i][1], // y
           scene_triangle[i][2], // z
           0xffff00ff,           // base_color
           end_of_strip);

    sq_transfer_32byte(ta_fifo_polygon_converter);
  }

  end_of_list(store_queue);
  sq_transfer_32byte(ta_fifo_polygon_converter);
  */
  uint32_t __attribute__((aligned(32))) scene[(32 * 5) / 4];
  triangle(&scene[(32 * 0) / 4]);
  for (int i = 0; i < 3; i++) {
    bool end_of_strip = i == 2;

    float x = scene_triangle[i][0];
    float y = scene_triangle[i][1];

    x = x * __builtin_cosf(theta) - y * __builtin_sinf(theta);
    y = x * __builtin_sinf(theta) + y * __builtin_cosf(theta);
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;

    vertex(&scene[(32 * (i + 1)) / 4],
           x, // x
           y, // y
           scene_triangle[i][2], // z
           0xffff00ff,           // base_color
           end_of_strip);
  }
  end_of_list(&scene[(32 * 4) / 4]);
  theta += degree;

  volatile uint32_t _dummy = sh7091.DMAC.CHCR2;
  (void)_dummy;
  sh7091.DMAC.CHCR2 = 0;
  sh7091.DMAC.SAR2 = reinterpret_cast<uint32_t>(&scene[0]);
  sh7091.DMAC.DMATCR2 = (32 * 5) / 32;
  //   SM(1) {source address increment}
  // | RS(2) {external request, single address mode}
  // | TM    {burst mode}
  // | TS(2) {32-byte block}
  // | DE    {enable channel}
  sh7091.DMAC.CHCR2 = 0x12c1;
  sh7091.DMAC.DMAOR = 0x8201;
  system.C2DSTAT = 0x10000000;
  system.C2DLEN = 32 * 5;
  system.C2DST = 1;

  while ((system.ISTNRM & (1 << 19)) == 0);
  system.ISTNRM = (1 << 19);
}

void scene_init_texture_memory()
{
  volatile texture_memory_alloc * mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory);

  background_parameter(mem->background);
  region_array(mem->region_array,
               (offsetof (struct texture_memory_alloc, object_list)),
               640 / 32, // width
               480 / 32  // height
               );
}

void scene_start_render(int fb)
{
  holly.REGION_BASE = (offsetof (struct texture_memory_alloc, region_array));
  holly.PARAM_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));

  holly.ISP_BACKGND_T = ISP_BACKGND_T__TAG_ADDRESS((offsetof (struct texture_memory_alloc, background)) / 4)
                      | ISP_BACKGND_T__TAG_OFFSET(0)
                      | ISP_BACKGND_T__SKIP(1);
  holly.ISP_BACKGND_D = _i(1.f/100000);

  //holly.SOFTRESET = softreset::pipeline_soft_reset;

  holly.FB_W_CTRL = 1 << 3 | FB_W_CTRL__FB_PACKMODE__565_RGB;
  holly.FB_W_LINESTRIDE = (640 * 2) / 8;

  int w_fb = (!(!fb)) * 0x00096000;
  int r_fb = (!fb) * 0x00096000;
  holly.FB_W_SOF1 = (offsetof (struct texture_memory_alloc, framebuffer)) + w_fb;
  holly.FB_R_SOF1 = (offsetof (struct texture_memory_alloc, framebuffer)) + r_fb;

  //holly.SOFTRESET = 0;

  holly.STARTRENDER = 1;
}
