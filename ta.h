#define ISP_FEED_CFG__TRANSLUCENCY_CACHE_SIZE(n) (((n) & 0x3ff) << 14)

#define FPU_SHAD_SCALE__INTENSITY_SHADOW_ENABLE (1 << 8)
#define FPU_SHAD_SCALE__SCALE_FACTOR(n) (((n) & 0xff) << 0)

#define FPU_CULL_VAL__COMPARISON_VALUE(n) (_u32(__builtin_fabsf(n)))

#define FPU_PERP_VAL__COMPARISON_VALUE(n) (_u32(__builtin_fabsf(n)))

#define SPAN_SORT_CFG__CACHE_BYPASS (1 << 16)
#define SPAN_SORT_CFG__OFFSET_SORT_ENABLE (1 << 8)
#define SPAN_SORT_CFG__SPAN_SORT_ENABLE (1 << 0)

#define FOG_COL_RAM__RED(n) (((n) & 0xff) << 16)
#define FOG_COL_RAM__GREEN(n) (((n) & 0xff) << 8)
#define FOG_COL_RAM__BLUE(n) (((n) & 0xff) << 0)

#define FOG_COL_VERT__RED(n)   (((n) & 0xff) << 16)
#define FOG_COL_VERT__GREEN(n) (((n) & 0xff) << 8)
#define FOG_COL_VERT__BLUE(n)  (((n) & 0xff) << 0)

#define FOG_CLAMP_MIN__ALPHA(n) (((n) & 0xff) << 24)
#define FOG_CLAMP_MIN__RED(n)   (((n) & 0xff) << 16)
#define FOG_CLAMP_MIN__GREEN(n) (((n) & 0xff) << 8)
#define FOG_CLAMP_MIN__BLUE(n)  (((n) & 0xff) << 0)

#define FOG_CLAMP_MAX__ALPHA(n) (((n) & 0xff) << 24)
#define FOG_CLAMP_MAX__RED(n)   (((n) & 0xff) << 16)
#define FOG_CLAMP_MAX__GREEN(n) (((n) & 0xff) << 8)
#define FOG_CLAMP_MAX__BLUE(n)  (((n) & 0xff) << 0)

#define HALF_OFFSET__TSP_TEXEL_SAMPLE_POSITION_CENTER (1 << 2)
#define HALF_OFFSET__TSP_PIXEL_SAMPLE_POSITION_CENTER (1 << 1)
#define HALF_OFFSET__FPU_PIXEL_SAMPLE_POSITION_CENTER (1 << 0)

#define FPU_PARAM_CFG__REGION_HEADER_TYPE__1 (0 << 21)
#define FPU_PARAM_CFG__REGION_HEADER_TYPE__2 (1 << 21)
#define FPU_PARAM_CFG__TSP_PARAMETER_BURST_TRIGGER(n) (((n) & 0x3f) << 14)
#define FPU_PARAM_CFG__ISP_PARAMETER_BURST_TRIGGER(n) (((n) & 0x3f) << 8)
#define FPU_PARAM_CFG__POINTER_BURST_SIZE(n)          (((n) & 0xf) << 4)
#define FPU_PARAM_CFG__POINTER_FIRST_BURST_SIZE(n)    (((n) & 0xf) << 0)

// --------------------

#define TA_GLOB_TILE_CLIP__TILE_Y_NUM(n) (((n) & 0b1111) << 16)
#define TA_GLOB_TILE_CLIP__TILE_X_NUM(n) (((n) & 0b11111) << 0)

#define TA_ALLOC_CTRL__OPB_MODE_INCREASING (0 << 20)
#define TA_ALLOC_CTRL__OPB_MODE_DECREASING (1 << 20)
#define TA_ALLOC_CTRL__PT_OPB__NONE (0b00 << 16)
#define TA_ALLOC_CTRL__PT_OPB__8    (0b01 << 16)
#define TA_ALLOC_CTRL__PT_OPB__16   (0b10 << 16)
#define TA_ALLOC_CTRL__PT_OPB__32   (0b11 << 16)
#define TA_ALLOC_CTRL__TM_OPB__NONE (0b00 << 12)
#define TA_ALLOC_CTRL__TM_OPB__8    (0b01 << 12)
#define TA_ALLOC_CTRL__TM_OPB__16   (0b10 << 12)
#define TA_ALLOC_CTRL__TM_OPB__32   (0b11 << 12)
#define TA_ALLOC_CTRL__T_OPB__NONE  (0b00 << 8)
#define TA_ALLOC_CTRL__T_OPB__8     (0b01 << 8)
#define TA_ALLOC_CTRL__T_OPB__16    (0b10 << 8)
#define TA_ALLOC_CTRL__T_OPB__32    (0b11 << 8)
#define TA_ALLOC_CTRL__OM_OPB__NONE (0b00 << 4)
#define TA_ALLOC_CTRL__OM_OPB__8    (0b01 << 4)
#define TA_ALLOC_CTRL__OM_OPB__16   (0b10 << 4)
#define TA_ALLOC_CTRL__OM_OPB__32   (0b11 << 4)
#define TA_ALLOC_CTRL__O_OPB__NONE  (0b00 << 0)
#define TA_ALLOC_CTRL__O_OPB__8     (0b01 << 0)
#define TA_ALLOC_CTRL__O_OPB__16    (0b10 << 0)
#define TA_ALLOC_CTRL__O_OPB__32    (0b11 << 0)

#define SOFTRESET__SDRAM_IF_SOFT_RESET (1 << 2)
#define SOFTRESET__CORE_SOFT_RESET (1 << 1)
#define SOFTRESET__TA_SOFT_RESET (1 << 0)

#define TA_LIST_INIT__LIST_INIT (1 << 31)

#define SB_ISTNRM__TAEOINT (1 << 7)  // End of Transferring interrupt : Opaque List

#define FB_W_CTRL__FB_PACKMODE__565_RGB (1 << 0);

#define ISP_BACKGND_T__CACHE_BYPASS (1 << 28)
#define ISP_BACKGND_T__SHADOW (1 << 27)
#define ISP_BACKGND_T__SKIP(n) (((n) & 0b111) << 24)
//#define ISP_BACKGND_T__TAG_ADDRESS(n) ((n) & 0xfffff8)
#define ISP_BACKGND_T__TAG_ADDRESS(n) (((n) & 0x3fffff) << 3)
#define ISP_BACKGND_T__TAG_OFFSET(n) (((n) & 0b111) << 0)

// ----------
