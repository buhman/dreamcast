#include <stdint.h>
#include <stddef.h>

struct holly_reg {
  uint32_t ID;             /* Device ID */
  uint32_t REVISION;       /* Revision Number */
  uint32_t SOFTRESET;      /* CORE & TA software reset */
  uint8_t  _pad0[8];
  uint32_t STARTRENDER;    /* Drawing start */
  uint32_t TEST_SELECT;    /* Test (writing this register is prohibited) */
  uint8_t  _pad1[4];
  uint32_t PARAM_BASE;     /* Base address for ISP parameters */
  uint8_t  _pad2[8];
  uint32_t REGION_BASE;    /* Base address for Region Array */
  uint32_t SPAN_SORT_CFG;  /* Span Sorter control */
  uint8_t  _pad3[12];
  uint32_t VO_BORDER_COL;  /* Border area color */
  uint32_t FB_R_CTRL;      /* Frame buffer read control */
  uint32_t FB_W_CTRL;      /* Frame buffer write control */
  uint32_t FB_W_LINESTRIDE;/* Frame buffer line stride */
  uint32_t FB_R_SOF1;      /* Read start address for field - 1/strip - 1 */
  uint32_t FB_R_SOF2;      /* Read start address for field - 2/strip - 2 */
  uint8_t  _pad4[4];
  uint32_t FB_R_SIZE;      /* Frame buffer XY size */
  uint32_t FB_W_SOF1;      /* Write start address for field - 1/strip - 1 */
  uint32_t FB_W_SOF2;      /* Write start address for field - 2/strip - 2 */
  uint32_t FB_X_CLIP;      /* Pixel clip X coordinate */
  uint32_t FB_Y_CLIP;      /* Pixel clip Y coordinate */
  uint8_t  _pad5[4];
  uint32_t FPU_SHAD_SCALE; /* Intensity Volume mode */
  uint32_t FPU_CULL_VAL;   /* Comparison value for culling */
  uint32_t FPU_PARAM_CFG;  /* Parameter read control */
  uint32_t HALF_OFFSET;    /* Pixel sampling control */
  uint32_t FPU_PERP_VAL;   /* Comparison value for perpendicular polygons */
  uint32_t ISP_BACKGND_D;  /* Background surface depth */
  uint32_t ISP_BACKGND_T;  /* Background surface tag */
  uint8_t  _pad6[8];
  uint32_t ISP_FEED_CFG;   /* Translucent polygon sort mode */
  uint8_t  _pad7[4];
  uint32_t SDRAM_REFRESH;  /* Texture memory refresh counter */
  uint32_t SDRAM_ARB_CFG;  /* Texture memory arbiter control */
  uint32_t SDRAM_CFG;      /* Texture memory control */
  uint8_t  _pad8[4];
  uint32_t FOG_COL_RAM;    /* Color for Look Up table Fog */
  uint32_t FOG_COL_VERT;   /* Color for vertex Fog */
  uint32_t FOG_DENSITY;    /* Fog scale value */
  uint32_t FOG_CLAMP_MAX;  /* Color clamping maximum value */
  uint32_t FOG_CLAMP_MIN;  /* Color clamping minimum value */
  uint32_t SPG_TRIGGER_POS;/* External trigger signal HV counter value */
  uint32_t SPG_HBLANK_INT; /* H-blank interrupt control */
  uint32_t SPG_VBLANK_INT; /* V-blank interrupt control */
  uint32_t SPG_CONTROL;    /* Sync pulse generator control */
  uint32_t SPG_HBLANK;     /* H-blank control */
  uint32_t SPG_LOAD;       /* HV counter load value */
  uint32_t SPG_VBLANK;     /* V-blank control */
  uint32_t SPG_WIDTH;      /* Sync width control */
  uint32_t TEXT_CONTROL;   /* Texturing control */
  uint32_t VO_CONTROL;     /* Video output control */
  uint32_t VO_STARTX;      /* Video output start X position */
  uint32_t VO_STARTY;      /* Video output start Y position */
  uint32_t SCALER_CTL;     /* X & Y scaler control */
  uint8_t  _pad9[16];
  uint32_t PAL_RAM_CTRL;   /* Palette RAM control */
  uint32_t SPG_STATUS;     /* Sync pulse generator status */
  uint32_t FB_BURSTCTRL;   /* Frame buffer burst control */
  uint32_t FB_C_SOF;       /* Current frame buffer start address */
  uint32_t Y_COEFF;        /* Y scaling coefficent */
  uint32_t PT_ALPHA_REF;   /* Alpha value for Punch Through polygon comparison */
  uint8_t  _pad10[4];
  uint32_t TA_OL_BASE;     /* Object List write start address */
  uint32_t TA_ISP_BASE;    /* ISP/TSP Parameter write start address */
  uint32_t TA_OL_LIMIT;    /* Object List write limit address */
  uint32_t TA_ISP_LIMIT;   /* ISP/TSP Parameter limit address */
  uint32_t TA_NEXT_OPB;    /* Start address for the Object Pointer Block */
  uint32_t TA_ITP_CURRENT; /* Starting address where the next ISP/TSP Parameters are stored */
  uint32_t TA_GLOB_TILE_CLIP;/* Global Tile Clip control */
  uint32_t TA_ALLOC_CTRL;  /* Object list control */
  uint32_t TA_LIST_INIT;   /* TA initialization */
  uint32_t TA_YUV_TEX_BASE;/* YUV422 texture write start address */
  uint32_t TA_YUV_TEX_CTRL;/* YUV converter control */
  uint32_t TA_YUV_TEX_CNT; /* YUV converter macro block counter value */
  uint8_t  _pad11[12];
  uint32_t TA_LIST_CONT;   /* TA continuation processing */
  uint32_t TA_NEXT_OPB_INIT;/* Additional OPB starting address */
  uint8_t  _pad12[152];
  uint8_t  FOG_TABLE[512]; /* Look-up table fog data */
  uint8_t  _pad13[512];
  uint8_t  TA_OL_POINTERS[2400];/* TA Object List Pointer data */
  uint8_t  _pad14[160];
  uint8_t  PALETTE_RAM[4096];/* Palette RAM */
};

static_assert((offsetof (struct holly_reg, ID)) == 0x0);
static_assert((offsetof (struct holly_reg, REVISION)) == 0x4);
static_assert((offsetof (struct holly_reg, SOFTRESET)) == 0x8);
static_assert((offsetof (struct holly_reg, STARTRENDER)) == 0x14);
static_assert((offsetof (struct holly_reg, TEST_SELECT)) == 0x18);
static_assert((offsetof (struct holly_reg, PARAM_BASE)) == 0x20);
static_assert((offsetof (struct holly_reg, REGION_BASE)) == 0x2c);
static_assert((offsetof (struct holly_reg, SPAN_SORT_CFG)) == 0x30);
static_assert((offsetof (struct holly_reg, VO_BORDER_COL)) == 0x40);
static_assert((offsetof (struct holly_reg, FB_R_CTRL)) == 0x44);
static_assert((offsetof (struct holly_reg, FB_W_CTRL)) == 0x48);
static_assert((offsetof (struct holly_reg, FB_W_LINESTRIDE)) == 0x4c);
static_assert((offsetof (struct holly_reg, FB_R_SOF1)) == 0x50);
static_assert((offsetof (struct holly_reg, FB_R_SOF2)) == 0x54);
static_assert((offsetof (struct holly_reg, FB_R_SIZE)) == 0x5c);
static_assert((offsetof (struct holly_reg, FB_W_SOF1)) == 0x60);
static_assert((offsetof (struct holly_reg, FB_W_SOF2)) == 0x64);
static_assert((offsetof (struct holly_reg, FB_X_CLIP)) == 0x68);
static_assert((offsetof (struct holly_reg, FB_Y_CLIP)) == 0x6c);
static_assert((offsetof (struct holly_reg, FPU_SHAD_SCALE)) == 0x74);
static_assert((offsetof (struct holly_reg, FPU_CULL_VAL)) == 0x78);
static_assert((offsetof (struct holly_reg, FPU_PARAM_CFG)) == 0x7c);
static_assert((offsetof (struct holly_reg, HALF_OFFSET)) == 0x80);
static_assert((offsetof (struct holly_reg, FPU_PERP_VAL)) == 0x84);
static_assert((offsetof (struct holly_reg, ISP_BACKGND_D)) == 0x88);
static_assert((offsetof (struct holly_reg, ISP_BACKGND_T)) == 0x8c);
static_assert((offsetof (struct holly_reg, ISP_FEED_CFG)) == 0x98);
static_assert((offsetof (struct holly_reg, SDRAM_REFRESH)) == 0xa0);
static_assert((offsetof (struct holly_reg, SDRAM_ARB_CFG)) == 0xa4);
static_assert((offsetof (struct holly_reg, SDRAM_CFG)) == 0xa8);
static_assert((offsetof (struct holly_reg, FOG_COL_RAM)) == 0xb0);
static_assert((offsetof (struct holly_reg, FOG_COL_VERT)) == 0xb4);
static_assert((offsetof (struct holly_reg, FOG_DENSITY)) == 0xb8);
static_assert((offsetof (struct holly_reg, FOG_CLAMP_MAX)) == 0xbc);
static_assert((offsetof (struct holly_reg, FOG_CLAMP_MIN)) == 0xc0);
static_assert((offsetof (struct holly_reg, SPG_TRIGGER_POS)) == 0xc4);
static_assert((offsetof (struct holly_reg, SPG_HBLANK_INT)) == 0xc8);
static_assert((offsetof (struct holly_reg, SPG_VBLANK_INT)) == 0xcc);
static_assert((offsetof (struct holly_reg, SPG_CONTROL)) == 0xd0);
static_assert((offsetof (struct holly_reg, SPG_HBLANK)) == 0xd4);
static_assert((offsetof (struct holly_reg, SPG_LOAD)) == 0xd8);
static_assert((offsetof (struct holly_reg, SPG_VBLANK)) == 0xdc);
static_assert((offsetof (struct holly_reg, SPG_WIDTH)) == 0xe0);
static_assert((offsetof (struct holly_reg, TEXT_CONTROL)) == 0xe4);
static_assert((offsetof (struct holly_reg, VO_CONTROL)) == 0xe8);
static_assert((offsetof (struct holly_reg, VO_STARTX)) == 0xec);
static_assert((offsetof (struct holly_reg, VO_STARTY)) == 0xf0);
static_assert((offsetof (struct holly_reg, SCALER_CTL)) == 0xf4);
static_assert((offsetof (struct holly_reg, PAL_RAM_CTRL)) == 0x108);
static_assert((offsetof (struct holly_reg, SPG_STATUS)) == 0x10c);
static_assert((offsetof (struct holly_reg, FB_BURSTCTRL)) == 0x110);
static_assert((offsetof (struct holly_reg, FB_C_SOF)) == 0x114);
static_assert((offsetof (struct holly_reg, Y_COEFF)) == 0x118);
static_assert((offsetof (struct holly_reg, PT_ALPHA_REF)) == 0x11c);
static_assert((offsetof (struct holly_reg, TA_OL_BASE)) == 0x124);
static_assert((offsetof (struct holly_reg, TA_ISP_BASE)) == 0x128);
static_assert((offsetof (struct holly_reg, TA_OL_LIMIT)) == 0x12c);
static_assert((offsetof (struct holly_reg, TA_ISP_LIMIT)) == 0x130);
static_assert((offsetof (struct holly_reg, TA_NEXT_OPB)) == 0x134);
static_assert((offsetof (struct holly_reg, TA_ITP_CURRENT)) == 0x138);
static_assert((offsetof (struct holly_reg, TA_GLOB_TILE_CLIP)) == 0x13c);
static_assert((offsetof (struct holly_reg, TA_ALLOC_CTRL)) == 0x140);
static_assert((offsetof (struct holly_reg, TA_LIST_INIT)) == 0x144);
static_assert((offsetof (struct holly_reg, TA_YUV_TEX_BASE)) == 0x148);
static_assert((offsetof (struct holly_reg, TA_YUV_TEX_CTRL)) == 0x14c);
static_assert((offsetof (struct holly_reg, TA_YUV_TEX_CNT)) == 0x150);
static_assert((offsetof (struct holly_reg, TA_LIST_CONT)) == 0x160);
static_assert((offsetof (struct holly_reg, TA_NEXT_OPB_INIT)) == 0x164);
static_assert((offsetof (struct holly_reg, FOG_TABLE)) == 0x200);
static_assert((offsetof (struct holly_reg, TA_OL_POINTERS)) == 0x600);
static_assert((offsetof (struct holly_reg, PALETTE_RAM)) == 0x1000);

extern holly_reg HOLLY;

