#pragma once

#include "reg.hpp"

namespace systembus {
  struct systembus_reg {
    reg32 C2DSTAT;             /* CH2-DMA destination address */
    reg32 C2DLEN;              /* CH2-DMA length */
    reg32 C2DST;               /* CH2-DMA start */
    reg8  _pad0[4];
    reg32 SDSTAW;              /* Sort-DMA start link table address */
    reg32 SDBAAW;              /* Sort-DMA link base address */
    reg32 SDWLT;               /* Sort-DMA link address bit width */
    reg32 SDLAS;               /* Sort-DMA link address shift control */
    reg32 SDST;                /* Sort-DMA start */
    reg8  _pad1[28];
    reg32 DBREQM;              /* DBREQ# signal mask control */
    reg32 BAVLWC;              /* BAVL# signal wait count */
    reg32 C2DPYRC;             /* DMA (TA/Root Bus) priority count */
    reg32 DMAXL;               /* CH2-DMA maximum burst length */
    reg8  _pad2[48];
    reg32 TFREM;               /* TA FIFO remaining amount */
    reg32 LMMODE0;             /* Via TA texture memory bus select 0 */
    reg32 LMMODE1;             /* Via TA texture memory bus select 1 */
    reg32 FFST;                /* FIFO status */
    reg32 SFRES;               /* System reset */
    reg8  _pad3[8];
    reg32 SBREV;               /* System bus revision number */
    reg32 RBSPLT;              /* SH4 Root Bus split enable */
    reg8  _pad4[92];
    reg32 ISTNRM;              /* Normal interrupt status */
    reg32 ISTEXT;              /* External interrupt status */
    reg32 ISTERR;              /* Error interrupt status */
    reg8  _pad5[4];
    reg32 IML2NRM;             /* Level 2 normal interrupt mask */
    reg32 IML2EXT;             /* Level 2 external interrupt mask */
    reg32 IML2ERR;             /* Level 2 error interrupt mask */
    reg8  _pad6[4];
    reg32 IML4NRM;             /* Level 4 normal interrupt mask */
    reg32 IML4EXT;             /* Level 4 external interrupt mask */
    reg32 IML4ERR;             /* Level 4 error interrupt mask */
    reg8  _pad7[4];
    reg32 IML6NRM;             /* Level 6 normal interrupt mask */
    reg32 IML6EXT;             /* Level 6 external interrupt mask */
    reg32 IML6ERR;             /* Level 6 error interrupt mask */
    reg8  _pad8[4];
    reg32 PDTNRM;              /* Normal interrupt PVR-DMA startup mask */
    reg32 PDTEXT;              /* External interrupt PVR-DMA startup mask */
    reg8  _pad9[8];
    reg32 G2DTNRM;             /* Normal interrupt G2-DMA startup mask */
    reg32 G2DTEXT;             /* External interrupt G2-DMA startup mask */
  };
  static_assert((offsetof (struct systembus_reg, C2DSTAT)) == 0x0);
  static_assert((offsetof (struct systembus_reg, C2DLEN)) == 0x4);
  static_assert((offsetof (struct systembus_reg, C2DST)) == 0x8);
  static_assert((offsetof (struct systembus_reg, SDSTAW)) == 0x10);
  static_assert((offsetof (struct systembus_reg, SDBAAW)) == 0x14);
  static_assert((offsetof (struct systembus_reg, SDWLT)) == 0x18);
  static_assert((offsetof (struct systembus_reg, SDLAS)) == 0x1c);
  static_assert((offsetof (struct systembus_reg, SDST)) == 0x20);
  static_assert((offsetof (struct systembus_reg, DBREQM)) == 0x40);
  static_assert((offsetof (struct systembus_reg, BAVLWC)) == 0x44);
  static_assert((offsetof (struct systembus_reg, C2DPYRC)) == 0x48);
  static_assert((offsetof (struct systembus_reg, DMAXL)) == 0x4c);
  static_assert((offsetof (struct systembus_reg, TFREM)) == 0x80);
  static_assert((offsetof (struct systembus_reg, LMMODE0)) == 0x84);
  static_assert((offsetof (struct systembus_reg, LMMODE1)) == 0x88);
  static_assert((offsetof (struct systembus_reg, FFST)) == 0x8c);
  static_assert((offsetof (struct systembus_reg, SFRES)) == 0x90);
  static_assert((offsetof (struct systembus_reg, SBREV)) == 0x9c);
  static_assert((offsetof (struct systembus_reg, RBSPLT)) == 0xa0);
  static_assert((offsetof (struct systembus_reg, ISTNRM)) == 0x100);
  static_assert((offsetof (struct systembus_reg, ISTEXT)) == 0x104);
  static_assert((offsetof (struct systembus_reg, ISTERR)) == 0x108);
  static_assert((offsetof (struct systembus_reg, IML2NRM)) == 0x110);
  static_assert((offsetof (struct systembus_reg, IML2EXT)) == 0x114);
  static_assert((offsetof (struct systembus_reg, IML2ERR)) == 0x118);
  static_assert((offsetof (struct systembus_reg, IML4NRM)) == 0x120);
  static_assert((offsetof (struct systembus_reg, IML4EXT)) == 0x124);
  static_assert((offsetof (struct systembus_reg, IML4ERR)) == 0x128);
  static_assert((offsetof (struct systembus_reg, IML6NRM)) == 0x130);
  static_assert((offsetof (struct systembus_reg, IML6EXT)) == 0x134);
  static_assert((offsetof (struct systembus_reg, IML6ERR)) == 0x138);
  static_assert((offsetof (struct systembus_reg, PDTNRM)) == 0x140);
  static_assert((offsetof (struct systembus_reg, PDTEXT)) == 0x144);
  static_assert((offsetof (struct systembus_reg, G2DTNRM)) == 0x150);
  static_assert((offsetof (struct systembus_reg, G2DTEXT)) == 0x154);

  struct maple_if_reg {
    reg8  _pad0[4];
    reg32 MDSTAR;              /* Maple-DMA command table address */
    reg8  _pad1[8];
    reg32 MDTSEL;              /* Maple-DMA trigger select */
    reg32 MDEN;                /* Maple-DMA enable */
    reg32 MDST;                /* Maple-DMA start */
    reg8  _pad2[100];
    reg32 MSYS;                /* Maple system control */
    reg32 MST;                 /* Maple status */
    reg32 MSHTCL;              /* Maple-DMA hard trigger clear */
    reg32 MDAPRO;              /* Maple-DMA address range */
    reg8  _pad3[88];
    reg32 MMSEL;               /* Maple MSP selection */
    reg8  _pad4[8];
    reg32 MTXDAD;              /* Maple TXD address counter */
    reg32 MRXDAD;              /* Maple RXD address counter */
    reg32 MRXDBD;              /* Maple RXD address base */
  };
  static_assert((offsetof (struct maple_if_reg, MDSTAR)) == 0x4);
  static_assert((offsetof (struct maple_if_reg, MDTSEL)) == 0x10);
  static_assert((offsetof (struct maple_if_reg, MDEN)) == 0x14);
  static_assert((offsetof (struct maple_if_reg, MDST)) == 0x18);
  static_assert((offsetof (struct maple_if_reg, MSYS)) == 0x80);
  static_assert((offsetof (struct maple_if_reg, MST)) == 0x84);
  static_assert((offsetof (struct maple_if_reg, MSHTCL)) == 0x88);
  static_assert((offsetof (struct maple_if_reg, MDAPRO)) == 0x8c);
  static_assert((offsetof (struct maple_if_reg, MMSEL)) == 0xe8);
  static_assert((offsetof (struct maple_if_reg, MTXDAD)) == 0xf4);
  static_assert((offsetof (struct maple_if_reg, MRXDAD)) == 0xf8);
  static_assert((offsetof (struct maple_if_reg, MRXDBD)) == 0xfc);

  struct g1_if_reg {
    reg8  _pad0[4];
    reg32 GDSTAR;              /* GD-DMA start address */
    reg32 GDLEN;               /* GD-DMA length */
    reg32 GDDIR;               /* GD-DMA direction */
    reg8  _pad1[4];
    reg32 GDEN;                /* GD-DMA enable */
    reg32 GDST;                /* GD-DMA start */
    reg8  _pad2[100];
    reg32 G1RRC;               /* System ROM read access timing */
    reg32 G1RWC;               /* System ROM write access timing */
    reg32 G1FRC;               /* Flash ROM read access timing */
    reg32 G1FWC;               /* Flash ROM write access timing */
    reg32 G1CRC;               /* GD PIO read access timing */
    reg32 G1CWC;               /* GD PIO write access timing */
    reg8  _pad3[8];
    reg32 G1GDRC;              /* GD-DMA read access timing */
    reg32 G1GDWC;              /* GD-DMA write access timing */
    reg8  _pad4[8];
    reg32 G1SYSM;              /* System mode */
    reg32 G1CRDYC;             /* G1IORDY signal control */
    reg32 GDAPRO;              /* GD-DMA address range */
    reg8  _pad5[40];
    reg32 GDUNLOCK;            /* (undocumented unlock register) */
    reg8  _pad6[12];
    reg32 GDSTARD;             /* GD-DMA address count (on Root Bus) */
    reg32 GDLEND;              /* GD-DMA transfer counter */
  };
  static_assert((offsetof (struct g1_if_reg, GDSTAR)) == 0x4);
  static_assert((offsetof (struct g1_if_reg, GDLEN)) == 0x8);
  static_assert((offsetof (struct g1_if_reg, GDDIR)) == 0xc);
  static_assert((offsetof (struct g1_if_reg, GDEN)) == 0x14);
  static_assert((offsetof (struct g1_if_reg, GDST)) == 0x18);
  static_assert((offsetof (struct g1_if_reg, G1RRC)) == 0x80);
  static_assert((offsetof (struct g1_if_reg, G1RWC)) == 0x84);
  static_assert((offsetof (struct g1_if_reg, G1FRC)) == 0x88);
  static_assert((offsetof (struct g1_if_reg, G1FWC)) == 0x8c);
  static_assert((offsetof (struct g1_if_reg, G1CRC)) == 0x90);
  static_assert((offsetof (struct g1_if_reg, G1CWC)) == 0x94);
  static_assert((offsetof (struct g1_if_reg, G1GDRC)) == 0xa0);
  static_assert((offsetof (struct g1_if_reg, G1GDWC)) == 0xa4);
  static_assert((offsetof (struct g1_if_reg, G1SYSM)) == 0xb0);
  static_assert((offsetof (struct g1_if_reg, G1CRDYC)) == 0xb4);
  static_assert((offsetof (struct g1_if_reg, GDAPRO)) == 0xb8);
  static_assert((offsetof (struct g1_if_reg, GDUNLOCK)) == 0xe4);
  static_assert((offsetof (struct g1_if_reg, GDSTARD)) == 0xf4);
  static_assert((offsetof (struct g1_if_reg, GDLEND)) == 0xf8);

  struct g2_if_reg {
    reg32 ADSTAG;              /* ACIA:G2-DMA G2 start address */
    reg32 ADSTAR;              /* ACIA:G2-DMA system memory start address */
    reg32 ADLEN;               /* ACIA:G2-DMA length */
    reg32 ADDIR;               /* ACIA:G2-DMA direction */
    reg32 ADTSEL;              /* ACIA:G2-DMA trigger select */
    reg32 ADEN;                /* ACIA:G2-DMA enable */
    reg32 ADST;                /* ACIA:G2-DMA start */
    reg32 ADSUSP;              /* ACIA:G2-DMA suspend */
    reg32 E1STAG;              /* Ext1:G2-DMA start address */
    reg32 E1STAR;              /* Ext1:G2-DMA system memory start address */
    reg32 E1LEN;               /* Ext1:G2-DMA length */
    reg32 E1DIR;               /* Ext1:G2-DMA direction */
    reg32 E1TSEL;              /* Ext1:G2-DMA trigger select */
    reg32 E1EN;                /* Ext1:G2-DMA enable */
    reg32 E1ST;                /* Ext1:G2-DMA start */
    reg32 E1SUSP;              /* Ext1:G2-DMA suspend */
    reg32 E2STAG;              /* Ext2:G2-DMA start address */
    reg32 E2STAR;              /* Ext2:G2-DMA system memory start address */
    reg32 E2LEN;               /* Ext2:G2-DMA length */
    reg32 E2DIR;               /* Ext2:G2-DMA direction */
    reg32 E2TSEL;              /* Ext2:G2-DMA trigger select */
    reg32 E2EN;                /* Ext2:G2-DMA enable */
    reg32 E2ST;                /* Ext2:G2-DMA start */
    reg32 E2SUSP;              /* Ext2:G2-DMA suspend */
    reg32 DDSTAG;              /* Dev:G2-DMA start address */
    reg32 DDSTAR;              /* Dev:G2-DMA system memory start address */
    reg32 DDLEN;               /* Dev:G2-DMA length */
    reg32 DDDIR;               /* Dev:G2-DMA direction */
    reg32 DDTSEL;              /* Dev:G2-DMA trigger select */
    reg32 DDEN;                /* Dev:G2-DMA enable */
    reg32 DDST;                /* Dev:G2-DMA start */
    reg32 DDSUSP;              /* Dev:G2-DMA suspend */
    reg32 G2ID;                /* G2 bus version */
    reg8  _pad0[12];
    reg32 G2DSTO;              /* G2/DS timeout */
    reg32 G2TRTO;              /* G2/TR timeout */
    reg32 G2MDMTO;             /* Modem unit wait timeout */
    reg32 G2MDMW;              /* Modem unit wait time */
    reg8  _pad1[28];
    reg32 G2APRO;              /* G2-DMA address range */
    reg32 ADSTAGD;             /* AICA-DMA address counter (on AICA) */
    reg32 ADSTARD;             /* AICA-DMA address counter (on root bus) */
    reg32 ADLEND;              /* AICA-DMA transfer counter */
    reg8  _pad2[4];
    reg32 E1STAGD;             /* Ext-DMA1 address counter (on Ext) */
    reg32 E1STARD;             /* Ext-DMA1 address counter (on root bus) */
    reg32 E1LEND;              /* Ext-DMA1 transfer counter */
    reg8  _pad3[4];
    reg32 E2STAGD;             /* Ext-DMA2 address counter (on Ext) */
    reg32 E2STARD;             /* Ext-DMA2 address counter (on root bus) */
    reg32 E2LEND;              /* Ext-DMA2 transfer counter */
    reg8  _pad4[4];
    reg32 DDSTAGD;             /* Dev-DMA address counter (on Dev) */
    reg32 DDSTARD;             /* Dev-DMA address counter (on root bus) */
    reg32 DDLEND;              /* Dev-DMA transfer counter */
  };
  static_assert((offsetof (struct g2_if_reg, ADSTAG)) == 0x0);
  static_assert((offsetof (struct g2_if_reg, ADSTAR)) == 0x4);
  static_assert((offsetof (struct g2_if_reg, ADLEN)) == 0x8);
  static_assert((offsetof (struct g2_if_reg, ADDIR)) == 0xc);
  static_assert((offsetof (struct g2_if_reg, ADTSEL)) == 0x10);
  static_assert((offsetof (struct g2_if_reg, ADEN)) == 0x14);
  static_assert((offsetof (struct g2_if_reg, ADST)) == 0x18);
  static_assert((offsetof (struct g2_if_reg, ADSUSP)) == 0x1c);
  static_assert((offsetof (struct g2_if_reg, E1STAG)) == 0x20);
  static_assert((offsetof (struct g2_if_reg, E1STAR)) == 0x24);
  static_assert((offsetof (struct g2_if_reg, E1LEN)) == 0x28);
  static_assert((offsetof (struct g2_if_reg, E1DIR)) == 0x2c);
  static_assert((offsetof (struct g2_if_reg, E1TSEL)) == 0x30);
  static_assert((offsetof (struct g2_if_reg, E1EN)) == 0x34);
  static_assert((offsetof (struct g2_if_reg, E1ST)) == 0x38);
  static_assert((offsetof (struct g2_if_reg, E1SUSP)) == 0x3c);
  static_assert((offsetof (struct g2_if_reg, E2STAG)) == 0x40);
  static_assert((offsetof (struct g2_if_reg, E2STAR)) == 0x44);
  static_assert((offsetof (struct g2_if_reg, E2LEN)) == 0x48);
  static_assert((offsetof (struct g2_if_reg, E2DIR)) == 0x4c);
  static_assert((offsetof (struct g2_if_reg, E2TSEL)) == 0x50);
  static_assert((offsetof (struct g2_if_reg, E2EN)) == 0x54);
  static_assert((offsetof (struct g2_if_reg, E2ST)) == 0x58);
  static_assert((offsetof (struct g2_if_reg, E2SUSP)) == 0x5c);
  static_assert((offsetof (struct g2_if_reg, DDSTAG)) == 0x60);
  static_assert((offsetof (struct g2_if_reg, DDSTAR)) == 0x64);
  static_assert((offsetof (struct g2_if_reg, DDLEN)) == 0x68);
  static_assert((offsetof (struct g2_if_reg, DDDIR)) == 0x6c);
  static_assert((offsetof (struct g2_if_reg, DDTSEL)) == 0x70);
  static_assert((offsetof (struct g2_if_reg, DDEN)) == 0x74);
  static_assert((offsetof (struct g2_if_reg, DDST)) == 0x78);
  static_assert((offsetof (struct g2_if_reg, DDSUSP)) == 0x7c);
  static_assert((offsetof (struct g2_if_reg, G2ID)) == 0x80);
  static_assert((offsetof (struct g2_if_reg, G2DSTO)) == 0x90);
  static_assert((offsetof (struct g2_if_reg, G2TRTO)) == 0x94);
  static_assert((offsetof (struct g2_if_reg, G2MDMTO)) == 0x98);
  static_assert((offsetof (struct g2_if_reg, G2MDMW)) == 0x9c);
  static_assert((offsetof (struct g2_if_reg, G2APRO)) == 0xbc);
  static_assert((offsetof (struct g2_if_reg, ADSTAGD)) == 0xc0);
  static_assert((offsetof (struct g2_if_reg, ADSTARD)) == 0xc4);
  static_assert((offsetof (struct g2_if_reg, ADLEND)) == 0xc8);
  static_assert((offsetof (struct g2_if_reg, E1STAGD)) == 0xd0);
  static_assert((offsetof (struct g2_if_reg, E1STARD)) == 0xd4);
  static_assert((offsetof (struct g2_if_reg, E1LEND)) == 0xd8);
  static_assert((offsetof (struct g2_if_reg, E2STAGD)) == 0xe0);
  static_assert((offsetof (struct g2_if_reg, E2STARD)) == 0xe4);
  static_assert((offsetof (struct g2_if_reg, E2LEND)) == 0xe8);
  static_assert((offsetof (struct g2_if_reg, DDSTAGD)) == 0xf0);
  static_assert((offsetof (struct g2_if_reg, DDSTARD)) == 0xf4);
  static_assert((offsetof (struct g2_if_reg, DDLEND)) == 0xf8);

  struct pvr_if_reg {
    reg32 PDSTAP;              /* PVR-DMA start address */
    reg32 PDSTAR;              /* PVR-DMA system memory start address */
    reg32 PDLEN;               /* PVR-DMA length */
    reg32 PDDIR;               /* PVR-DMA direction */
    reg32 PDTSEL;              /* PVR-DMA trigger select */
    reg32 PDEN;                /* PVR-DMA enable */
    reg32 PDST;                /* PVR-DMA start */
    reg8  _pad0[100];
    reg32 PDAPRO;              /* PVR-DMA address range */
    reg8  _pad1[108];
    reg32 PDSTAPD;             /* PVR-DMA address counter (on Ext) */
    reg32 PDSTARD;             /* PVR-DMA address counter (on root bus) */
    reg32 PDLEND;              /* PVR-DMA transfer counter */
  };
  static_assert((offsetof (struct pvr_if_reg, PDSTAP)) == 0x0);
  static_assert((offsetof (struct pvr_if_reg, PDSTAR)) == 0x4);
  static_assert((offsetof (struct pvr_if_reg, PDLEN)) == 0x8);
  static_assert((offsetof (struct pvr_if_reg, PDDIR)) == 0xc);
  static_assert((offsetof (struct pvr_if_reg, PDTSEL)) == 0x10);
  static_assert((offsetof (struct pvr_if_reg, PDEN)) == 0x14);
  static_assert((offsetof (struct pvr_if_reg, PDST)) == 0x18);
  static_assert((offsetof (struct pvr_if_reg, PDAPRO)) == 0x80);
  static_assert((offsetof (struct pvr_if_reg, PDSTAPD)) == 0xf0);
  static_assert((offsetof (struct pvr_if_reg, PDSTARD)) == 0xf4);
  static_assert((offsetof (struct pvr_if_reg, PDLEND)) == 0xf8);

  extern struct systembus_reg systembus __asm("systembus");
  extern struct maple_if_reg maple_if __asm("maple_if");
  extern struct g1_if_reg g1_if __asm("g1_if");
  extern struct g2_if_reg g2_if __asm("g2_if");
  extern struct pvr_if_reg pvr_if __asm("pvr_if");
}
