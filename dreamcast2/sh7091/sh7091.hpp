#pragma once

#include <cstdint>
#include <cstddef>

#include "type.hpp"

struct ccn_reg {
  reg32 PTEH;                /* Page table entry high register */
  reg32 PTEL;                /* Page table entry low register */
  reg32 TTB;                 /* Translation table base register */
  reg32 TEA;                 /* TLB exception address register */
  reg32 MMUCR;               /* MMU control register */
  reg8  BASRA;               /* Break ASID register A */
  reg8  _pad0[3];
  reg8  BASRB;               /* Break ASID register B */
  reg8  _pad1[3];
  reg32 CCR;                 /* Cache control register */
  reg32 TRA;                 /* TRAPA exception register */
  reg32 EXPEVT;              /* Exception event register */
  reg32 INTEVT;              /* Interrupt event register */
  reg8  _pad2[8];
  reg32 PTEA;                /* Page table entry assistance register */
  reg32 QACR0;               /* Queue address control register 0 */
  reg32 QACR1;               /* Queue address control register 1 */
};

static_assert((offsetof (struct ccn_reg, PTEH)) == 0x0);
static_assert((offsetof (struct ccn_reg, PTEL)) == 0x4);
static_assert((offsetof (struct ccn_reg, TTB)) == 0x8);
static_assert((offsetof (struct ccn_reg, TEA)) == 0xc);
static_assert((offsetof (struct ccn_reg, MMUCR)) == 0x10);
static_assert((offsetof (struct ccn_reg, BASRA)) == 0x14);
static_assert((offsetof (struct ccn_reg, BASRB)) == 0x18);
static_assert((offsetof (struct ccn_reg, CCR)) == 0x1c);
static_assert((offsetof (struct ccn_reg, TRA)) == 0x20);
static_assert((offsetof (struct ccn_reg, EXPEVT)) == 0x24);
static_assert((offsetof (struct ccn_reg, INTEVT)) == 0x28);
static_assert((offsetof (struct ccn_reg, PTEA)) == 0x34);
static_assert((offsetof (struct ccn_reg, QACR0)) == 0x38);
static_assert((offsetof (struct ccn_reg, QACR1)) == 0x3c);

struct ubc_reg {
  reg32 BARA;                /* Break address register A */
  reg8  BAMRA;               /* Break address mask register A */
  reg8  _pad0[3];
  reg16 BBRA;                /* Break bus cycle register A */
  reg8  _pad1[2];
  reg32 BARB;                /* Break address register B */
  reg8  BAMRB;               /* Break address mask register B */
  reg8  _pad2[3];
  reg16 BBRB;                /* Break bus cycle register B */
  reg8  _pad3[2];
  reg32 BDRB;                /* Break data register B */
  reg32 BDMRB;               /* Break data mask register B */
  reg16 BRCR;                /* Break control register */
};

static_assert((offsetof (struct ubc_reg, BARA)) == 0x0);
static_assert((offsetof (struct ubc_reg, BAMRA)) == 0x4);
static_assert((offsetof (struct ubc_reg, BBRA)) == 0x8);
static_assert((offsetof (struct ubc_reg, BARB)) == 0xc);
static_assert((offsetof (struct ubc_reg, BAMRB)) == 0x10);
static_assert((offsetof (struct ubc_reg, BBRB)) == 0x14);
static_assert((offsetof (struct ubc_reg, BDRB)) == 0x18);
static_assert((offsetof (struct ubc_reg, BDMRB)) == 0x1c);
static_assert((offsetof (struct ubc_reg, BRCR)) == 0x20);

struct bsc_reg {
  reg32 BCR1;                /* Bus control register 1 */
  reg16 BCR2;                /* Bus control register 2 */
  reg8  _pad0[2];
  reg32 WCR1;                /* Wait state control register 1 */
  reg32 WCR2;                /* Wait state control register 2 */
  reg32 WCR3;                /* Wait state control register 3 */
  reg32 MCR;                 /* Memory control register */
  reg16 PCR;                 /* PCMCIA control register */
  reg8  _pad1[2];
  reg16 RTCSR;               /* Refresh timer control/status register */
  reg8  _pad2[2];
  reg16 RTCNT;               /* Refresh timer counter */
  reg8  _pad3[2];
  reg16 RTCOR;               /* Refresh timer constant counter */
  reg8  _pad4[2];
  reg16 RFCR;                /* Refresh count register */
  reg8  _pad5[2];
  reg32 PCTRA;               /* Port control register A */
  reg16 PDTRA;               /* Port data register A */
  reg8  _pad6[14];
  reg32 PCTRB;               /* Port control register B */
  reg16 PDTRB;               /* Port data register B */
  reg8  _pad7[2];
  reg16 GPIOIC;              /* GPIO interrupt control register */
  reg8  _pad8[1048502];
  reg32 SDMR2[16384];        /* Synchronous DRAM mode registers */
  reg8  _pad9[196608];
  reg32 SDMR3[16384];        /* Synchronous DRAM mode registers */
};

static_assert((offsetof (struct bsc_reg, BCR1)) == 0x0);
static_assert((offsetof (struct bsc_reg, BCR2)) == 0x4);
static_assert((offsetof (struct bsc_reg, WCR1)) == 0x8);
static_assert((offsetof (struct bsc_reg, WCR2)) == 0xc);
static_assert((offsetof (struct bsc_reg, WCR3)) == 0x10);
static_assert((offsetof (struct bsc_reg, MCR)) == 0x14);
static_assert((offsetof (struct bsc_reg, PCR)) == 0x18);
static_assert((offsetof (struct bsc_reg, RTCSR)) == 0x1c);
static_assert((offsetof (struct bsc_reg, RTCNT)) == 0x20);
static_assert((offsetof (struct bsc_reg, RTCOR)) == 0x24);
static_assert((offsetof (struct bsc_reg, RFCR)) == 0x28);
static_assert((offsetof (struct bsc_reg, PCTRA)) == 0x2c);
static_assert((offsetof (struct bsc_reg, PDTRA)) == 0x30);
static_assert((offsetof (struct bsc_reg, PCTRB)) == 0x40);
static_assert((offsetof (struct bsc_reg, PDTRB)) == 0x44);
static_assert((offsetof (struct bsc_reg, GPIOIC)) == 0x48);
static_assert((offsetof (struct bsc_reg, SDMR2)) == 0x100000);
static_assert((offsetof (struct bsc_reg, SDMR3)) == 0x140000);

struct dmac_reg {
  reg32 SAR0;                /* DMA source address register 0 */
  reg32 DAR0;                /* DMA destination address register 0 */
  reg32 DMATCR0;             /* DMA transfer count register 0 */
  reg32 CHCR0;               /* DMA control register 0 */
  reg32 SAR1;                /* DMA source address register 1 */
  reg32 DAR1;                /* DMA destination address register 1 */
  reg32 DMATCR1;             /* DMA transfer count register 1 */
  reg32 CHCR1;               /* DMA control register 1 */
  reg32 SAR2;                /* DMA source address register 2 */
  reg32 DAR2;                /* DMA destination address register 2 */
  reg32 DMATCR2;             /* DMA transfer count register 2 */
  reg32 CHCR2;               /* DMA control register 2 */
  reg32 SAR3;                /* DMA source address register 3 */
  reg32 DAR3;                /* DMA destination address register 3 */
  reg32 DMATCR3;             /* DMA transfer count register 3 */
  reg32 CHCR3;               /* DMA control register 3 */
  reg32 DMAOR;               /* DMA operation register */
};

static_assert((offsetof (struct dmac_reg, SAR0)) == 0x0);
static_assert((offsetof (struct dmac_reg, DAR0)) == 0x4);
static_assert((offsetof (struct dmac_reg, DMATCR0)) == 0x8);
static_assert((offsetof (struct dmac_reg, CHCR0)) == 0xc);
static_assert((offsetof (struct dmac_reg, SAR1)) == 0x10);
static_assert((offsetof (struct dmac_reg, DAR1)) == 0x14);
static_assert((offsetof (struct dmac_reg, DMATCR1)) == 0x18);
static_assert((offsetof (struct dmac_reg, CHCR1)) == 0x1c);
static_assert((offsetof (struct dmac_reg, SAR2)) == 0x20);
static_assert((offsetof (struct dmac_reg, DAR2)) == 0x24);
static_assert((offsetof (struct dmac_reg, DMATCR2)) == 0x28);
static_assert((offsetof (struct dmac_reg, CHCR2)) == 0x2c);
static_assert((offsetof (struct dmac_reg, SAR3)) == 0x30);
static_assert((offsetof (struct dmac_reg, DAR3)) == 0x34);
static_assert((offsetof (struct dmac_reg, DMATCR3)) == 0x38);
static_assert((offsetof (struct dmac_reg, CHCR3)) == 0x3c);
static_assert((offsetof (struct dmac_reg, DMAOR)) == 0x40);

struct cpg_reg {
  reg16 FRQCR;               /* Frequency control register */
  reg8  _pad0[2];
  reg8  STBCR;               /* Standby control register */
  reg8  _pad1[3];
  reg16 WTCNT;               /* Watchdog timer counter */
  reg8  _pad2[2];
  reg16 WTCSR;               /* Watchdog timer control/status register */
  reg8  _pad3[2];
  reg8  STBCR2;              /* Standby control register 2 */
};

static_assert((offsetof (struct cpg_reg, FRQCR)) == 0x0);
static_assert((offsetof (struct cpg_reg, STBCR)) == 0x4);
static_assert((offsetof (struct cpg_reg, WTCNT)) == 0x8);
static_assert((offsetof (struct cpg_reg, WTCSR)) == 0xc);
static_assert((offsetof (struct cpg_reg, STBCR2)) == 0x10);

struct rtc_reg {
  reg8  R64CNT;              /* 64 Hz counter */
  reg8  _pad0[3];
  reg8  RSECCNT;             /* Second counter */
  reg8  _pad1[3];
  reg8  RMINCNT;             /* Minute counter */
  reg8  _pad2[3];
  reg8  RHRCNT;              /* Hour counter */
  reg8  _pad3[3];
  reg8  RWKCNT;              /* Day-of-week counter */
  reg8  _pad4[3];
  reg8  RDAYCNT;             /* Day counter */
  reg8  _pad5[3];
  reg8  RMONCNT;             /* Month counter */
  reg8  _pad6[3];
  reg16 RYRCNT;              /* Year counter */
  reg8  _pad7[2];
  reg8  RSECAR;              /* Second alarm register */
  reg8  _pad8[3];
  reg8  RMINAR;              /* Minute alarm register */
  reg8  _pad9[3];
  reg8  RHRAR;               /* Hour alarm register */
  reg8  _pad10[3];
  reg8  RWKAR;               /* Day-of-week alarm register */
  reg8  _pad11[3];
  reg8  RDAYAR;              /* Day alarm register */
  reg8  _pad12[3];
  reg8  RMONAR;              /* Month alarm register */
  reg8  _pad13[3];
  reg8  RCR1;                /* RTC control register 1 */
  reg8  _pad14[3];
  reg8  RCR2;                /* RTC control register 2 */
};

static_assert((offsetof (struct rtc_reg, R64CNT)) == 0x0);
static_assert((offsetof (struct rtc_reg, RSECCNT)) == 0x4);
static_assert((offsetof (struct rtc_reg, RMINCNT)) == 0x8);
static_assert((offsetof (struct rtc_reg, RHRCNT)) == 0xc);
static_assert((offsetof (struct rtc_reg, RWKCNT)) == 0x10);
static_assert((offsetof (struct rtc_reg, RDAYCNT)) == 0x14);
static_assert((offsetof (struct rtc_reg, RMONCNT)) == 0x18);
static_assert((offsetof (struct rtc_reg, RYRCNT)) == 0x1c);
static_assert((offsetof (struct rtc_reg, RSECAR)) == 0x20);
static_assert((offsetof (struct rtc_reg, RMINAR)) == 0x24);
static_assert((offsetof (struct rtc_reg, RHRAR)) == 0x28);
static_assert((offsetof (struct rtc_reg, RWKAR)) == 0x2c);
static_assert((offsetof (struct rtc_reg, RDAYAR)) == 0x30);
static_assert((offsetof (struct rtc_reg, RMONAR)) == 0x34);
static_assert((offsetof (struct rtc_reg, RCR1)) == 0x38);
static_assert((offsetof (struct rtc_reg, RCR2)) == 0x3c);

struct intc_reg {
  reg16 ICR;                 /* Interrupt control register */
  reg8  _pad0[2];
  reg16 IPRA;                /* Interrupt priority register A */
  reg8  _pad1[2];
  reg16 IPRB;                /* Interrupt priority register B */
  reg8  _pad2[2];
  reg16 IPRC;                /* Interrupt priority register C */
};

static_assert((offsetof (struct intc_reg, ICR)) == 0x0);
static_assert((offsetof (struct intc_reg, IPRA)) == 0x4);
static_assert((offsetof (struct intc_reg, IPRB)) == 0x8);
static_assert((offsetof (struct intc_reg, IPRC)) == 0xc);

struct tmu_reg {
  reg8  TOCR;                /* Timer output control register */
  reg8  _pad0[3];
  reg8  TSTR;                /* Timer start register */
  reg8  _pad1[3];
  reg32 TCOR0;               /* Timer constant register 0 */
  reg32 TCNT0;               /* Timer counter 0 */
  reg16 TCR0;                /* Timer control register 0 */
  reg8  _pad2[2];
  reg32 TCOR1;               /* Timer constant register 1 */
  reg32 TCNT1;               /* Timer counter 1 */
  reg16 TCR1;                /* Timer control register 1 */
  reg8  _pad3[2];
  reg32 TCOR2;               /* Timer constant register 2 */
  reg32 TCNT2;               /* Timer counter 2 */
  reg16 TCR2;                /* Timer control register 2 */
  reg8  _pad4[2];
  reg32 TCPR2;               /* Timer input capture register 2 */
};

static_assert((offsetof (struct tmu_reg, TOCR)) == 0x0);
static_assert((offsetof (struct tmu_reg, TSTR)) == 0x4);
static_assert((offsetof (struct tmu_reg, TCOR0)) == 0x8);
static_assert((offsetof (struct tmu_reg, TCNT0)) == 0xc);
static_assert((offsetof (struct tmu_reg, TCR0)) == 0x10);
static_assert((offsetof (struct tmu_reg, TCOR1)) == 0x14);
static_assert((offsetof (struct tmu_reg, TCNT1)) == 0x18);
static_assert((offsetof (struct tmu_reg, TCR1)) == 0x1c);
static_assert((offsetof (struct tmu_reg, TCOR2)) == 0x20);
static_assert((offsetof (struct tmu_reg, TCNT2)) == 0x24);
static_assert((offsetof (struct tmu_reg, TCR2)) == 0x28);
static_assert((offsetof (struct tmu_reg, TCPR2)) == 0x2c);

struct sci_reg {
  reg8  SCSMR1;              /* Serial mode register 1 */
  reg8  _pad0[3];
  reg8  SCBRR1;              /* Bit rate register 1 */
  reg8  _pad1[3];
  reg8  SCSCR1;              /* Serial control register 1 */
  reg8  _pad2[3];
  reg8  SCTDR1;              /* Transmit data register 1 */
  reg8  _pad3[3];
  reg8  SCSSR1;              /* Serial status register 1 */
  reg8  _pad4[3];
  reg8  SCRDR1;              /* Receive data register 1 */
  reg8  _pad5[3];
  reg8  SCSCMR1;             /* Smart card mode register 1 */
  reg8  _pad6[3];
  reg8  SCSPTR1;             /* Serial port register */
};

static_assert((offsetof (struct sci_reg, SCSMR1)) == 0x0);
static_assert((offsetof (struct sci_reg, SCBRR1)) == 0x4);
static_assert((offsetof (struct sci_reg, SCSCR1)) == 0x8);
static_assert((offsetof (struct sci_reg, SCTDR1)) == 0xc);
static_assert((offsetof (struct sci_reg, SCSSR1)) == 0x10);
static_assert((offsetof (struct sci_reg, SCRDR1)) == 0x14);
static_assert((offsetof (struct sci_reg, SCSCMR1)) == 0x18);
static_assert((offsetof (struct sci_reg, SCSPTR1)) == 0x1c);

struct scif_reg {
  reg16 SCSMR2;              /* Serial mode register 2 */
  reg8  _pad0[2];
  reg8  SCBRR2;              /* Bit rate register 2 */
  reg8  _pad1[3];
  reg16 SCSCR2;              /* Serial control register 2 */
  reg8  _pad2[2];
  reg8  SCFTDR2;             /* Transmit FIFO data register 2 */
  reg8  _pad3[3];
  reg16 SCFSR2;              /* Serial status register 2 */
  reg8  _pad4[2];
  reg8  SCFRDR2;             /* Receive FIFO data register 2 */
  reg8  _pad5[3];
  reg16 SCFCR2;              /* FIFO control register */
  reg8  _pad6[2];
  reg16 SCFDR2;              /* FIFO data count register */
  reg8  _pad7[2];
  reg16 SCSPTR2;             /* Serial port register 2 */
  reg8  _pad8[2];
  reg16 SCLSR2;              /* Line status register 2 */
};

static_assert((offsetof (struct scif_reg, SCSMR2)) == 0x0);
static_assert((offsetof (struct scif_reg, SCBRR2)) == 0x4);
static_assert((offsetof (struct scif_reg, SCSCR2)) == 0x8);
static_assert((offsetof (struct scif_reg, SCFTDR2)) == 0xc);
static_assert((offsetof (struct scif_reg, SCFSR2)) == 0x10);
static_assert((offsetof (struct scif_reg, SCFRDR2)) == 0x14);
static_assert((offsetof (struct scif_reg, SCFCR2)) == 0x18);
static_assert((offsetof (struct scif_reg, SCFDR2)) == 0x1c);
static_assert((offsetof (struct scif_reg, SCSPTR2)) == 0x20);
static_assert((offsetof (struct scif_reg, SCLSR2)) == 0x24);

struct udi_reg {
  reg16 SDIR;                /* Instruction register */
  reg8  _pad0[6];
  reg32 SDDR;                /* Data register */
};

static_assert((offsetof (struct udi_reg, SDIR)) == 0x0);
static_assert((offsetof (struct udi_reg, SDDR)) == 0x8);

struct sh7091_reg {
  struct ccn_reg CCN;
  reg8  _pad0[0x200000 - (sizeof (struct ccn_reg))];
  struct ubc_reg UBC;
  reg8  _pad1[0x600000 - (sizeof (struct ubc_reg))];
  struct bsc_reg BSC;
  reg8  _pad2[0x200000 - (sizeof (struct bsc_reg))];
  struct dmac_reg DMAC;
  reg8  _pad3[0x200000 - (sizeof (struct dmac_reg))];
  struct cpg_reg CPG;
  reg8  _pad4[0x80000 - (sizeof (struct cpg_reg))];
  struct rtc_reg RTC;
  reg8  _pad5[0x80000 - (sizeof (struct rtc_reg))];
  struct intc_reg INTC;
  reg8  _pad6[0x80000 - (sizeof (struct intc_reg))];
  struct tmu_reg TMU;
  reg8  _pad7[0x80000 - (sizeof (struct tmu_reg))];
  struct sci_reg SCI;
  reg8  _pad8[0x80000 - (sizeof (struct sci_reg))];
  struct scif_reg SCIF;
  reg8  _pad9[0x80000 - (sizeof (struct scif_reg))];
  struct udi_reg UDI;
};

static_assert((offsetof (struct sh7091_reg, CCN)) == 0x0);
static_assert((offsetof (struct sh7091_reg, UBC)) == 0x200000);
static_assert((offsetof (struct sh7091_reg, BSC)) == 0x800000);
static_assert((offsetof (struct sh7091_reg, DMAC)) == 0xa00000);
static_assert((offsetof (struct sh7091_reg, CPG)) == 0xc00000);
static_assert((offsetof (struct sh7091_reg, RTC)) == 0xc80000);
static_assert((offsetof (struct sh7091_reg, INTC)) == 0xd00000);
static_assert((offsetof (struct sh7091_reg, TMU)) == 0xd80000);
static_assert((offsetof (struct sh7091_reg, SCI)) == 0xe00000);
static_assert((offsetof (struct sh7091_reg, SCIF)) == 0xe80000);
static_assert((offsetof (struct sh7091_reg, UDI)) == 0xf00000);

extern struct sh7091_reg sh7091 __asm("sh7091");
