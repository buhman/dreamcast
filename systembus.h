#include <stdint.h>
#include <stddef.h>

struct system_reg {
  uint32_t C2DSTAT;        /* CH2-DMA destination address */
  uint32_t C2DLEN;         /* CH2-DMA length */
  uint32_t C2DST;          /* CH2-DMA start */
  uint8_t  _pad0[4];
  uint32_t SDSTAW;         /* Sort-DMA start link table address */
  uint32_t SDBAAW;         /* Sort-DMA link base address */
  uint32_t SDWLT;          /* Sort-DMA link address bit width */
  uint32_t SDLAS;          /* Sort-DMA link address shift control */
  uint32_t SDST;           /* Sort-DMA start */
  uint8_t  _pad1[28];
  uint32_t DBREQM;         /* DBREQ# signal mask control */
  uint32_t BAVLWC;         /* BAVL# signal wait count */
  uint32_t C2DPYRC;        /* DMA (TA/Root Bus) priority count */
  uint32_t DMAXL;          /* CH2-DMA maximum burst length */
  uint8_t  _pad2[48];
  uint32_t TFREM;          /* TA FIFO remaining amount */
  uint32_t LMMODE0;        /* Via TA texture memory bus select 0 */
  uint32_t LMMODE1;        /* Via TA texture memory bus select 1 */
  uint32_t FFST;           /* FIFO status */
  uint32_t SFRES;          /* System reset */
  uint8_t  _pad3[8];
  uint32_t SBREV;          /* System bus revision number */
  uint32_t RBSPLT;         /* SH4 Root Bus split enable */
  uint8_t  _pad4[92];
  uint32_t ISTNRM;         /* Normal interrupt status */
  uint32_t ISTEXT;         /* External interrupt status */
  uint32_t ISTERR;         /* Error interrupt status */
  uint8_t  _pad5[4];
  uint32_t IML2NRM;        /* Level 2 normal interrupt mask */
  uint32_t IML2EXT;        /* Level 2 external interrupt mask */
  uint32_t IML2ERR;        /* Level 2 error interrupt mask */
  uint8_t  _pad6[4];
  uint32_t IML4NRM;        /* Level 4 normal interrupt mask */
  uint32_t IML4EXT;        /* Level 4 external interrupt mask */
  uint32_t IML4ERR;        /* Level 4 error interrupt mask */
  uint8_t  _pad7[4];
  uint32_t IML6NRM;        /* Level 6 normal interrupt mask */
  uint32_t IML6EXT;        /* Level 6 external interrupt mask */
  uint32_t IML6ERR;        /* Level 6 error interrupt mask */
  uint8_t  _pad8[4];
  uint32_t PDTNRM;         /* Normal interrupt PVR-DMA startup mask */
  uint32_t PDTEXT;         /* External interrupt PVR-DMA startup mask */
  uint8_t  _pad9[8];
  uint32_t G2DTNRM;        /* Normal interrupt G2-DMA startup mask */
  uint32_t G2DTEXT;        /* External interrupt G2-DMA startup mask */
};

static_assert((offsetof (struct system_reg, C2DSTAT)) == 0x0);
static_assert((offsetof (struct system_reg, C2DLEN)) == 0x4);
static_assert((offsetof (struct system_reg, C2DST)) == 0x8);
static_assert((offsetof (struct system_reg, SDSTAW)) == 0x10);
static_assert((offsetof (struct system_reg, SDBAAW)) == 0x14);
static_assert((offsetof (struct system_reg, SDWLT)) == 0x18);
static_assert((offsetof (struct system_reg, SDLAS)) == 0x1c);
static_assert((offsetof (struct system_reg, SDST)) == 0x20);
static_assert((offsetof (struct system_reg, DBREQM)) == 0x40);
static_assert((offsetof (struct system_reg, BAVLWC)) == 0x44);
static_assert((offsetof (struct system_reg, C2DPYRC)) == 0x48);
static_assert((offsetof (struct system_reg, DMAXL)) == 0x4c);
static_assert((offsetof (struct system_reg, TFREM)) == 0x80);
static_assert((offsetof (struct system_reg, LMMODE0)) == 0x84);
static_assert((offsetof (struct system_reg, LMMODE1)) == 0x88);
static_assert((offsetof (struct system_reg, FFST)) == 0x8c);
static_assert((offsetof (struct system_reg, SFRES)) == 0x90);
static_assert((offsetof (struct system_reg, SBREV)) == 0x9c);
static_assert((offsetof (struct system_reg, RBSPLT)) == 0xa0);
static_assert((offsetof (struct system_reg, ISTNRM)) == 0x100);
static_assert((offsetof (struct system_reg, ISTEXT)) == 0x104);
static_assert((offsetof (struct system_reg, ISTERR)) == 0x108);
static_assert((offsetof (struct system_reg, IML2NRM)) == 0x110);
static_assert((offsetof (struct system_reg, IML2EXT)) == 0x114);
static_assert((offsetof (struct system_reg, IML2ERR)) == 0x118);
static_assert((offsetof (struct system_reg, IML4NRM)) == 0x120);
static_assert((offsetof (struct system_reg, IML4EXT)) == 0x124);
static_assert((offsetof (struct system_reg, IML4ERR)) == 0x128);
static_assert((offsetof (struct system_reg, IML6NRM)) == 0x130);
static_assert((offsetof (struct system_reg, IML6EXT)) == 0x134);
static_assert((offsetof (struct system_reg, IML6ERR)) == 0x138);
static_assert((offsetof (struct system_reg, PDTNRM)) == 0x140);
static_assert((offsetof (struct system_reg, PDTEXT)) == 0x144);
static_assert((offsetof (struct system_reg, G2DTNRM)) == 0x150);
static_assert((offsetof (struct system_reg, G2DTEXT)) == 0x154);

struct maple_reg {
  uint32_t MDSTAR;         /* Maple-DMA command table address */
  uint8_t  _pad0[8];
  uint32_t MDTSEL;         /* Maple-DMA trigger select */
  uint32_t MDEN;           /* Maple-DMA enable */
  uint32_t MDST;           /* Maple-DMA start */
  uint8_t  _pad1[100];
  uint32_t MSYS;           /* Maple system control */
  uint32_t MST;            /* Maple status */
  uint32_t MSHTCL;         /* Maple-DMA hard trigger clear */
  uint32_t MDAPRO;         /* Maple-DMA address range */
  uint8_t  _pad2[88];
  uint32_t MMSEL;          /* Maple MSP selection */
  uint8_t  _pad3[8];
  uint32_t MTXDAD;         /* Maple TXD address counter */
  uint32_t MRXDAD;         /* Maple RXD address counter */
  uint32_t MRXDBD;         /* Maple RXD address base */
};

static_assert((offsetof (struct maple_reg, MDSTAR)) == 0x0);
static_assert((offsetof (struct maple_reg, MDTSEL)) == 0xc);
static_assert((offsetof (struct maple_reg, MDEN)) == 0x10);
static_assert((offsetof (struct maple_reg, MDST)) == 0x14);
static_assert((offsetof (struct maple_reg, MSYS)) == 0x7c);
static_assert((offsetof (struct maple_reg, MST)) == 0x80);
static_assert((offsetof (struct maple_reg, MSHTCL)) == 0x84);
static_assert((offsetof (struct maple_reg, MDAPRO)) == 0x88);
static_assert((offsetof (struct maple_reg, MMSEL)) == 0xe4);
static_assert((offsetof (struct maple_reg, MTXDAD)) == 0xf0);
static_assert((offsetof (struct maple_reg, MRXDAD)) == 0xf4);
static_assert((offsetof (struct maple_reg, MRXDBD)) == 0xf8);

struct g1_reg {
  uint32_t GDSTAR;         /* GD-DMA start address */
  uint32_t GDLEN;          /* GD-DMA length */
  uint32_t GDDIR;          /* GD-DMA direction */
  uint8_t  _pad0[4];
  uint32_t GDEN;           /* GD-DMA enable */
  uint32_t GDST;           /* GD-DMA start */
  uint8_t  _pad1[100];
  uint32_t G1RRC;          /* System ROM read access timing */
  uint32_t G1RWC;          /* System ROM write access timing */
  uint32_t G1FRC;          /* Flash ROM read access timing */
  uint32_t G1FWC;          /* Flash ROM write access timing */
  uint32_t G1CRC;          /* GD PIO read access timing */
  uint32_t G1CWC;          /* GD PIO write access timing */
  uint8_t  _pad2[8];
  uint32_t G1GDRC;         /* GD-DMA read access timing */
  uint32_t G1GDWC;         /* GD-DMA write access timing */
  uint8_t  _pad3[8];
  uint32_t G1SYSM;         /* System mode */
  uint32_t G1CRDYC;        /* G1IORDY signal control */
  uint32_t GDAPRO;         /* GD-DMA address range */
  uint8_t  _pad4[56];
  uint32_t GDSTARD;        /* GD-DMA address count (on Root Bus) */
  uint32_t GDLEND;         /* GD-DMA transfer counter */
};

static_assert((offsetof (struct g1_reg, GDSTAR)) == 0x0);
static_assert((offsetof (struct g1_reg, GDLEN)) == 0x4);
static_assert((offsetof (struct g1_reg, GDDIR)) == 0x8);
static_assert((offsetof (struct g1_reg, GDEN)) == 0x10);
static_assert((offsetof (struct g1_reg, GDST)) == 0x14);
static_assert((offsetof (struct g1_reg, G1RRC)) == 0x7c);
static_assert((offsetof (struct g1_reg, G1RWC)) == 0x80);
static_assert((offsetof (struct g1_reg, G1FRC)) == 0x84);
static_assert((offsetof (struct g1_reg, G1FWC)) == 0x88);
static_assert((offsetof (struct g1_reg, G1CRC)) == 0x8c);
static_assert((offsetof (struct g1_reg, G1CWC)) == 0x90);
static_assert((offsetof (struct g1_reg, G1GDRC)) == 0x9c);
static_assert((offsetof (struct g1_reg, G1GDWC)) == 0xa0);
static_assert((offsetof (struct g1_reg, G1SYSM)) == 0xac);
static_assert((offsetof (struct g1_reg, G1CRDYC)) == 0xb0);
static_assert((offsetof (struct g1_reg, GDAPRO)) == 0xb4);
static_assert((offsetof (struct g1_reg, GDSTARD)) == 0xf0);
static_assert((offsetof (struct g1_reg, GDLEND)) == 0xf4);

struct g2_reg {
  uint32_t ADSTAG;         /* ACIA:G2-DMA G2 start address */
  uint32_t ADSTAR;         /* ACIA:G2-DMA system memory start address */
  uint32_t ADLEN;          /* ACIA:G2-DMA length */
  uint32_t ADDIR;          /* ACIA:G2-DMA direction */
  uint32_t ADTSEL;         /* ACIA:G2-DMA trigger select */
  uint32_t ADEN;           /* ACIA:G2-DMA enable */
  uint32_t ADST;           /* ACIA:G2-DMA start */
  uint32_t ADSUSP;         /* ACIA:G2-DMA suspend */
  uint32_t E1STAG;         /* Ext1:G2-DMA start address */
  uint32_t E1STAR;         /* Ext1:G2-DMA system memory start address */
  uint32_t E1LEN;          /* Ext1:G2-DMA length */
  uint32_t E1DIR;          /* Ext1:G2-DMA direction */
  uint32_t E1TSEL;         /* Ext1:G2-DMA trigger select */
  uint32_t E1EN;           /* Ext1:G2-DMA enable */
  uint32_t E1ST;           /* Ext1:G2-DMA start */
  uint32_t E1SUSP;         /* Ext1:G2-DMA suspend */
  uint32_t E2STAG;         /* Ext2:G2-DMA start address */
  uint32_t E2STAR;         /* Ext2:G2-DMA system memory start address */
  uint32_t E2LEN;          /* Ext2:G2-DMA length */
  uint32_t E2DIR;          /* Ext2:G2-DMA direction */
  uint32_t E2TSEL;         /* Ext2:G2-DMA trigger select */
  uint32_t E2EN;           /* Ext2:G2-DMA enable */
  uint32_t E2ST;           /* Ext2:G2-DMA start */
  uint32_t E2SUSP;         /* Ext2:G2-DMA suspend */
  uint32_t DDSTAG;         /* Dev:G2-DMA start address */
  uint32_t DDSTAR;         /* Dev:G2-DMA system memory start address */
  uint32_t DDLEN;          /* Dev:G2-DMA length */
  uint32_t DDDIR;          /* Dev:G2-DMA direction */
  uint32_t DDTSEL;         /* Dev:G2-DMA trigger select */
  uint32_t DDEN;           /* Dev:G2-DMA enable */
  uint32_t DDST;           /* Dev:G2-DMA start */
  uint32_t DDSUSP;         /* Dev:G2-DMA suspend */
  uint32_t G2ID;           /* G2 bus version */
  uint8_t  _pad0[12];
  uint32_t G2DSTO;         /* G2/DS timeout */
  uint32_t G2TRTO;         /* G2/TR timeout */
  uint32_t G2MDMTO;        /* Modem unit wait timeout */
  uint32_t G2MDMW;         /* Modem unit wait time */
  uint8_t  _pad1[28];
  uint32_t G2APRO;         /* G2-DMA address range */
  uint32_t ADSTAGD;        /* AICA-DMA address counter (on AICA) */
  uint32_t ADSTARD;        /* AICA-DMA address counter (on root bus) */
  uint32_t ADLEND;         /* AICA-DMA transfer counter */
  uint8_t  _pad2[4];
  uint32_t E1STAGD;        /* Ext-DMA1 address counter (on Ext) */
  uint32_t E1STARD;        /* Ext-DMA1 address counter (on root bus) */
  uint32_t E1LEND;         /* Ext-DMA1 transfer counter */
  uint8_t  _pad3[4];
  uint32_t E2STAGD;        /* Ext-DMA2 address counter (on Ext) */
  uint32_t E2STARD;        /* Ext-DMA2 address counter (on root bus) */
  uint32_t E2LEND;         /* Ext-DMA2 transfer counter */
  uint8_t  _pad4[4];
  uint32_t DDSTAGD;        /* Dev-DMA address counter (on Dev) */
  uint32_t DDSTARD;        /* Dev-DMA address counter (on root bus) */
  uint32_t DDLEND;         /* Dev-DMA transfer counter */
};

static_assert((offsetof (struct g2_reg, ADSTAG)) == 0x0);
static_assert((offsetof (struct g2_reg, ADSTAR)) == 0x4);
static_assert((offsetof (struct g2_reg, ADLEN)) == 0x8);
static_assert((offsetof (struct g2_reg, ADDIR)) == 0xc);
static_assert((offsetof (struct g2_reg, ADTSEL)) == 0x10);
static_assert((offsetof (struct g2_reg, ADEN)) == 0x14);
static_assert((offsetof (struct g2_reg, ADST)) == 0x18);
static_assert((offsetof (struct g2_reg, ADSUSP)) == 0x1c);
static_assert((offsetof (struct g2_reg, E1STAG)) == 0x20);
static_assert((offsetof (struct g2_reg, E1STAR)) == 0x24);
static_assert((offsetof (struct g2_reg, E1LEN)) == 0x28);
static_assert((offsetof (struct g2_reg, E1DIR)) == 0x2c);
static_assert((offsetof (struct g2_reg, E1TSEL)) == 0x30);
static_assert((offsetof (struct g2_reg, E1EN)) == 0x34);
static_assert((offsetof (struct g2_reg, E1ST)) == 0x38);
static_assert((offsetof (struct g2_reg, E1SUSP)) == 0x3c);
static_assert((offsetof (struct g2_reg, E2STAG)) == 0x40);
static_assert((offsetof (struct g2_reg, E2STAR)) == 0x44);
static_assert((offsetof (struct g2_reg, E2LEN)) == 0x48);
static_assert((offsetof (struct g2_reg, E2DIR)) == 0x4c);
static_assert((offsetof (struct g2_reg, E2TSEL)) == 0x50);
static_assert((offsetof (struct g2_reg, E2EN)) == 0x54);
static_assert((offsetof (struct g2_reg, E2ST)) == 0x58);
static_assert((offsetof (struct g2_reg, E2SUSP)) == 0x5c);
static_assert((offsetof (struct g2_reg, DDSTAG)) == 0x60);
static_assert((offsetof (struct g2_reg, DDSTAR)) == 0x64);
static_assert((offsetof (struct g2_reg, DDLEN)) == 0x68);
static_assert((offsetof (struct g2_reg, DDDIR)) == 0x6c);
static_assert((offsetof (struct g2_reg, DDTSEL)) == 0x70);
static_assert((offsetof (struct g2_reg, DDEN)) == 0x74);
static_assert((offsetof (struct g2_reg, DDST)) == 0x78);
static_assert((offsetof (struct g2_reg, DDSUSP)) == 0x7c);
static_assert((offsetof (struct g2_reg, G2ID)) == 0x80);
static_assert((offsetof (struct g2_reg, G2DSTO)) == 0x90);
static_assert((offsetof (struct g2_reg, G2TRTO)) == 0x94);
static_assert((offsetof (struct g2_reg, G2MDMTO)) == 0x98);
static_assert((offsetof (struct g2_reg, G2MDMW)) == 0x9c);
static_assert((offsetof (struct g2_reg, G2APRO)) == 0xbc);
static_assert((offsetof (struct g2_reg, ADSTAGD)) == 0xc0);
static_assert((offsetof (struct g2_reg, ADSTARD)) == 0xc4);
static_assert((offsetof (struct g2_reg, ADLEND)) == 0xc8);
static_assert((offsetof (struct g2_reg, E1STAGD)) == 0xd0);
static_assert((offsetof (struct g2_reg, E1STARD)) == 0xd4);
static_assert((offsetof (struct g2_reg, E1LEND)) == 0xd8);
static_assert((offsetof (struct g2_reg, E2STAGD)) == 0xe0);
static_assert((offsetof (struct g2_reg, E2STARD)) == 0xe4);
static_assert((offsetof (struct g2_reg, E2LEND)) == 0xe8);
static_assert((offsetof (struct g2_reg, DDSTAGD)) == 0xf0);
static_assert((offsetof (struct g2_reg, DDSTARD)) == 0xf4);
static_assert((offsetof (struct g2_reg, DDLEND)) == 0xf8);

struct pvr_reg {
  uint32_t PDSTAP;         /* PVR-DMA start address */
  uint32_t PDSTAR;         /* PVR-DMA system memory start address */
  uint32_t PDLEN;          /* PVR-DMA length */
  uint32_t PDDIR;          /* PVR-DMA direction */
  uint32_t PDTSEL;         /* PVR-DMA trigger select */
  uint32_t PDEN;           /* PVR-DMA enable */
  uint32_t PDST;           /* PVR-DMA start */
  uint8_t  _pad0[100];
  uint32_t PDAPRO;         /* PVR-DMA address range */
  uint8_t  _pad1[108];
  uint32_t PDSTAPD;        /* PVR-DMA address counter (on Ext) */
  uint32_t PDSTARD;        /* PVR-DMA address counter (on root bus) */
  uint32_t PDLEND;         /* PVR-DMA transfer counter */
};

static_assert((offsetof (struct pvr_reg, PDSTAP)) == 0x0);
static_assert((offsetof (struct pvr_reg, PDSTAR)) == 0x4);
static_assert((offsetof (struct pvr_reg, PDLEN)) == 0x8);
static_assert((offsetof (struct pvr_reg, PDDIR)) == 0xc);
static_assert((offsetof (struct pvr_reg, PDTSEL)) == 0x10);
static_assert((offsetof (struct pvr_reg, PDEN)) == 0x14);
static_assert((offsetof (struct pvr_reg, PDST)) == 0x18);
static_assert((offsetof (struct pvr_reg, PDAPRO)) == 0x80);
static_assert((offsetof (struct pvr_reg, PDSTAPD)) == 0xf0);
static_assert((offsetof (struct pvr_reg, PDSTARD)) == 0xf4);
static_assert((offsetof (struct pvr_reg, PDLEND)) == 0xf8);

