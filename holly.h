#include <stdint.h>
#include <stddef.h>

#include "type.h"

struct holly_reg {
  reg32 ID;                /* Device ID */
  reg32 REVISION;          /* Revision Number */
  reg32 SOFTRESET;         /* CORE & TA software reset */
  reg8  _pad0[8];
  reg32 STARTRENDER;       /* Drawing start */
  reg32 TEST_SELECT;       /* Test (writing this register is prohibited) */
  reg8  _pad1[4];
  reg32 PARAM_BASE;        /* Base address for ISP parameters */
  reg8  _pad2[8];
  reg32 REGION_BASE;       /* Base address for Region Array */
  reg32 SPAN_SORT_CFG;     /* Span Sorter control */
  reg8  _pad3[12];
  reg32 VO_BORDER_COL;     /* Border area color */
  reg32 FB_R_CTRL;         /* Frame buffer read control */
  reg32 FB_W_CTRL;         /* Frame buffer write control */
  reg32 FB_W_LINESTRIDE;   /* Frame buffer line stride */
  reg32 FB_R_SOF1;         /* Read start address for field - 1/strip - 1 */
  reg32 FB_R_SOF2;         /* Read start address for field - 2/strip - 2 */
  reg8  _pad4[4];
  reg32 FB_R_SIZE;         /* Frame buffer XY size */
  reg32 FB_W_SOF1;         /* Write start address for field - 1/strip - 1 */
  reg32 FB_W_SOF2;         /* Write start address for field - 2/strip - 2 */
  reg32 FB_X_CLIP;         /* Pixel clip X coordinate */
  reg32 FB_Y_CLIP;         /* Pixel clip Y coordinate */
  reg8  _pad5[4];
  reg32 FPU_SHAD_SCALE;    /* Intensity Volume mode */
  reg32 FPU_CULL_VAL;      /* Comparison value for culling */
  reg32 FPU_PARAM_CFG;     /* Parameter read control */
  reg32 HALF_OFFSET;       /* Pixel sampling control */
  reg32 FPU_PERP_VAL;      /* Comparison value for perpendicular polygons */
  reg32 ISP_BACKGND_D;     /* Background surface depth */
  reg32 ISP_BACKGND_T;     /* Background surface tag */
  reg8  _pad6[8];
  reg32 ISP_FEED_CFG;      /* Translucent polygon sort mode */
  reg8  _pad7[4];
  reg32 SDRAM_REFRESH;     /* Texture memory refresh counter */
  reg32 SDRAM_ARB_CFG;     /* Texture memory arbiter control */
  reg32 SDRAM_CFG;         /* Texture memory control */
  reg8  _pad8[4];
  reg32 FOG_COL_RAM;       /* Color for Look Up table Fog */
  reg32 FOG_COL_VERT;      /* Color for vertex Fog */
  reg32 FOG_DENSITY;       /* Fog scale value */
  reg32 FOG_CLAMP_MAX;     /* Color clamping maximum value */
  reg32 FOG_CLAMP_MIN;     /* Color clamping minimum value */
  reg32 SPG_TRIGGER_POS;   /* External trigger signal HV counter value */
  reg32 SPG_HBLANK_INT;    /* H-blank interrupt control */
  reg32 SPG_VBLANK_INT;    /* V-blank interrupt control */
  reg32 SPG_CONTROL;       /* Sync pulse generator control */
  reg32 SPG_HBLANK;        /* H-blank control */
  reg32 SPG_LOAD;          /* HV counter load value */
  reg32 SPG_VBLANK;        /* V-blank control */
  reg32 SPG_WIDTH;         /* Sync width control */
  reg32 TEXT_CONTROL;      /* Texturing control */
  reg32 VO_CONTROL;        /* Video output control */
  reg32 VO_STARTX;         /* Video output start X position */
  reg32 VO_STARTY;         /* Video output start Y position */
  reg32 SCALER_CTL;        /* X & Y scaler control */
  reg8  _pad9[16];
  reg32 PAL_RAM_CTRL;      /* Palette RAM control */
  reg32 SPG_STATUS;        /* Sync pulse generator status */
  reg32 FB_BURSTCTRL;      /* Frame buffer burst control */
  reg32 FB_C_SOF;          /* Current frame buffer start address */
  reg32 Y_COEFF;           /* Y scaling coefficent */
  reg32 PT_ALPHA_REF;      /* Alpha value for Punch Through polygon comparison */
  reg8  _pad10[4];
  reg32 TA_OL_BASE;        /* Object List write start address */
  reg32 TA_ISP_BASE;       /* ISP/TSP Parameter write start address */
  reg32 TA_OL_LIMIT;       /* Object List write limit address */
  reg32 TA_ISP_LIMIT;      /* ISP/TSP Parameter limit address */
  reg32 TA_NEXT_OPB;       /* Start address for the Object Pointer Block */
  reg32 TA_ITP_CURRENT;    /* Starting address where the next ISP/TSP Parameters are stored */
  reg32 TA_GLOB_TILE_CLIP; /* Global Tile Clip control */
  reg32 TA_ALLOC_CTRL;     /* Object list control */
  reg32 TA_LIST_INIT;      /* TA initialization */
  reg32 TA_YUV_TEX_BASE;   /* YUV422 texture write start address */
  reg32 TA_YUV_TEX_CTRL;   /* YUV converter control */
  reg32 TA_YUV_TEX_CNT;    /* YUV converter macro block counter value */
  reg8  _pad11[12];
  reg32 TA_LIST_CONT;      /* TA continuation processing */
  reg32 TA_NEXT_OPB_INIT;  /* Additional OPB starting address */
  reg8  _pad12[152];
  reg8  FOG_TABLE[512];    /* Look-up table fog data */
  reg8  _pad13[512];
  reg8  TA_OL_POINTERS[2400];/* TA Object List Pointer data */
  reg8  _pad14[160];
  reg8  PALETTE_RAM[4096]; /* Palette RAM */
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

extern struct holly_reg holly __asm("holly");
