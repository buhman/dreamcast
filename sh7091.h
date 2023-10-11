#include <cstdint>
#include <cstddef>

struct ccn_reg {
  uint32_t PTEH;           /* Page table entry high register */
  uint32_t PTEL;           /* Page table entry low register */
  uint32_t TTB;            /* Translation table base register */
  uint32_t TEA;            /* TLB exception address register */
  uint32_t MMUCR;          /* MMU control register */
  uint8_t  BASRA;          /* Break ASID register A */
  uint8_t  _pad0[3];
  uint8_t  BASRB;          /* Break ASID register B */
  uint8_t  _pad1[3];
  uint32_t CCR;            /* Cache control register */
  uint32_t TRA;            /* TRAPA exception register */
  uint32_t EXPEVT;         /* Exception event register */
  uint32_t INTEVT;         /* Interrupt event register */
  uint8_t  _pad2[8];
  uint32_t PTEA;           /* Page table entry assistance register */
  uint32_t QACR0;          /* Queue address control register 0 */
  uint32_t QACR1;          /* Queue address control register 1 */
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
  uint32_t BARA;           /* Break address register A */
  uint8_t  BAMRA;          /* Break address mask register A */
  uint8_t  _pad0[3];
  uint16_t BBRA;           /* Break bus cycle register A */
  uint8_t  _pad1[2];
  uint32_t BARB;           /* Break address register B */
  uint8_t  BAMRB;          /* Break address mask register B */
  uint8_t  _pad2[3];
  uint16_t BBRB;           /* Break bus cycle register B */
  uint8_t  _pad3[2];
  uint32_t BDRB;           /* Break data register B */
  uint32_t BDMRB;          /* Break data mask register B */
  uint16_t BRCR;           /* Break control register */
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
  uint32_t BCR1;           /* Bus control register 1 */
  uint16_t BCR2;           /* Bus control register 2 */
  uint8_t  _pad0[2];
  uint32_t WCR1;           /* Wait state control register 1 */
  uint32_t WCR2;           /* Wait state control register 2 */
  uint32_t WCR3;           /* Wait state control register 3 */
  uint32_t MCR;            /* Memory control register */
  uint16_t PCR;            /* PCMCIA control register */
  uint8_t  _pad1[2];
  uint16_t RTCSR;          /* Refresh timer control/status register */
  uint8_t  _pad2[2];
  uint16_t RTCNT;          /* Refresh timer counter */
  uint8_t  _pad3[2];
  uint16_t RTCOR;          /* Refresh timer constant counter */
  uint8_t  _pad4[2];
  uint16_t RFCR;           /* Refresh count register */
  uint8_t  _pad5[2];
  uint32_t PCTRA;          /* Port control register A */
  uint16_t PDTRA;          /* Port data register A */
  uint8_t  _pad6[14];
  uint32_t PCTRB;          /* Port control register B */
  uint16_t PDTRB;          /* Port data register B */
  uint8_t  _pad7[2];
  uint16_t GPIOIC;         /* GPIO interrupt control register */
  uint8_t  _pad8[1048502];
  uint8_t  SDMR2[65536];   /* Synchronous DRAM mode registers */
  uint8_t  _pad9[196608];
  uint8_t  SDMR3[65536];   /* Synchronous DRAM mode registers */
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
  uint32_t SAR0;           /* DMA source address register 0 */
  uint32_t DAR0;           /* DMA destination address register 0 */
  uint32_t DMATCR0;        /* DMA transfer count register 0 */
  uint32_t CHCR0;          /* DMA control register 0 */
  uint32_t SAR1;           /* DMA source address register 1 */
  uint32_t DAR1;           /* DMA destination address register 1 */
  uint32_t DMATCR1;        /* DMA transfer count register 1 */
  uint32_t CHCR1;          /* DMA control register 1 */
  uint32_t SAR2;           /* DMA source address register 2 */
  uint32_t DAR2;           /* DMA destination address register 2 */
  uint32_t DMATCR2;        /* DMA transfer count register 2 */
  uint32_t CHCR2;          /* DMA control register 2 */
  uint32_t SAR3;           /* DMA source address register 3 */
  uint32_t DAR3;           /* DMA destination address register 3 */
  uint32_t DMATCR3;        /* DMA transfer count register 3 */
  uint32_t CHCR3;          /* DMA control register 3 */
  uint32_t DMAOR;          /* DMA operation register */
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
  uint16_t FRQCR;          /* Frequency control register */
  uint8_t  _pad0[2];
  uint8_t  STBCR;          /* Standby control register */
  uint8_t  _pad1[3];
  uint16_t WTCNT;          /* Watchdog timer counter */
  uint8_t  _pad2[2];
  uint16_t WTCSR;          /* Watchdog timer control/status register */
  uint8_t  _pad3[2];
  uint8_t  STBCR2;         /* Standby control register 2 */
};

static_assert((offsetof (struct cpg_reg, FRQCR)) == 0x0);
static_assert((offsetof (struct cpg_reg, STBCR)) == 0x4);
static_assert((offsetof (struct cpg_reg, WTCNT)) == 0x8);
static_assert((offsetof (struct cpg_reg, WTCSR)) == 0xc);
static_assert((offsetof (struct cpg_reg, STBCR2)) == 0x10);

struct rtc_reg {
  uint8_t  R64CNT;         /* 64 Hz counter */
  uint8_t  _pad0[3];
  uint8_t  RSECCNT;        /* Second counter */
  uint8_t  _pad1[3];
  uint8_t  RMINCNT;        /* Minute counter */
  uint8_t  _pad2[3];
  uint8_t  RHRCNT;         /* Hour counter */
  uint8_t  _pad3[3];
  uint8_t  RWKCNT;         /* Day-of-week counter */
  uint8_t  _pad4[3];
  uint8_t  RDAYCNT;        /* Day counter */
  uint8_t  _pad5[3];
  uint8_t  RMONCNT;        /* Month counter */
  uint8_t  _pad6[3];
  uint16_t RYRCNT;         /* Year counter */
  uint8_t  _pad7[2];
  uint8_t  RSECAR;         /* Second alarm register */
  uint8_t  _pad8[3];
  uint8_t  RMINAR;         /* Minute alarm register */
  uint8_t  _pad9[3];
  uint8_t  RHRAR;          /* Hour alarm register */
  uint8_t  _pad10[3];
  uint8_t  RWKAR;          /* Day-of-week alarm register */
  uint8_t  _pad11[3];
  uint8_t  RDAYAR;         /* Day alarm register */
  uint8_t  _pad12[3];
  uint8_t  RMONAR;         /* Month alarm register */
  uint8_t  _pad13[3];
  uint8_t  RCR1;           /* RTC control register 1 */
  uint8_t  _pad14[3];
  uint8_t  RCR2;           /* RTC control register 2 */
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
  uint16_t ICR;            /* Interrupt control register */
  uint8_t  _pad0[2];
  uint16_t IPRA;           /* Interrupt priority register A */
  uint8_t  _pad1[2];
  uint16_t IPRB;           /* Interrupt priority register B */
  uint8_t  _pad2[2];
  uint16_t IPRC;           /* Interrupt priority register C */
};

static_assert((offsetof (struct intc_reg, ICR)) == 0x0);
static_assert((offsetof (struct intc_reg, IPRA)) == 0x4);
static_assert((offsetof (struct intc_reg, IPRB)) == 0x8);
static_assert((offsetof (struct intc_reg, IPRC)) == 0xc);

struct tmu_reg {
  uint8_t  TOCR;           /* Timer output control register */
  uint8_t  _pad0[3];
  uint8_t  TSTR;           /* Timer start register */
  uint8_t  _pad1[3];
  uint32_t TCOR0;          /* Timer constant register 0 */
  uint32_t TCNT0;          /* Timer counter 0 */
  uint16_t TCR0;           /* Timer control register 0 */
  uint8_t  _pad2[2];
  uint32_t TCOR1;          /* Timer constant register 1 */
  uint32_t TCNT1;          /* Timer counter 1 */
  uint16_t TCR1;           /* Timer control register 1 */
  uint8_t  _pad3[2];
  uint32_t TCOR2;          /* Timer constant register 2 */
  uint32_t TCNT2;          /* Timer counter 2 */
  uint16_t TCR2;           /* Timer control register 2 */
  uint8_t  _pad4[2];
  uint32_t TCPR2;          /* Timer input capture register 2 */
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
  uint8_t  SCSMR1;         /* Serial mode register 1 */
  uint8_t  _pad0[3];
  uint8_t  SCBRR1;         /* Bit rate register 1 */
  uint8_t  _pad1[3];
  uint8_t  SCSCR1;         /* Serial control register 1 */
  uint8_t  _pad2[3];
  uint8_t  SCTDR1;         /* Transmit data register 1 */
  uint8_t  _pad3[3];
  uint8_t  SCSSR1;         /* Serial status register 1 */
  uint8_t  _pad4[3];
  uint8_t  SCRDR1;         /* Receive data register 1 */
  uint8_t  _pad5[3];
  uint8_t  SCSCMR1;        /* Smart card mode register 1 */
  uint8_t  _pad6[3];
  uint8_t  SCSPTR1;        /* Serial port register */
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
  uint16_t SCSMR2;         /* Serial mode register 2 */
  uint8_t  _pad0[2];
  uint8_t  SCBRR2;         /* Bit rate register 2 */
  uint8_t  _pad1[3];
  uint16_t SCSCR2;         /* Serial control register 2 */
  uint8_t  _pad2[2];
  uint8_t  SCFTDR2;        /* Transmit FIFO data register 2 */
  uint8_t  _pad3[3];
  uint16_t SCFSR2;         /* Serial status register 2 */
  uint8_t  _pad4[2];
  uint8_t  SCFRDR2;        /* Receive FIFO data register 2 */
  uint8_t  _pad5[3];
  uint16_t SCFCR2;         /* FIFO control register */
  uint8_t  _pad6[2];
  uint16_t SCFDR2;         /* FIFO data count register */
  uint8_t  _pad7[2];
  uint16_t SCSPTR2;        /* Serial port register 2 */
  uint8_t  _pad8[2];
  uint16_t SCLSR2;         /* Line status register 2 */
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
  uint16_t SDIR;           /* Instruction register */
  uint8_t  _pad0[6];
  uint32_t SDDR;           /* Data register */
};

static_assert((offsetof (struct udi_reg, SDIR)) == 0x0);
static_assert((offsetof (struct udi_reg, SDDR)) == 0x8);

struct sh7091_reg {
  ccn_reg CCN;
  uint8_t  _pad0[0x200000 - (sizeof (struct ccn_reg))];
  ubc_reg UBC;
  uint8_t  _pad1[0x600000 - (sizeof (struct ubc_reg))];
  bsc_reg BSC;
  uint8_t  _pad2[0x200000 - (sizeof (struct bsc_reg))];
  dmac_reg DMAC;
  uint8_t  _pad3[0x200000 - (sizeof (struct dmac_reg))];
  cpg_reg CPG;
  uint8_t  _pad4[0x80000 - (sizeof (struct cpg_reg))];
  rtc_reg RTC;
  uint8_t  _pad5[0x80000 - (sizeof (struct rtc_reg))];
  intc_reg INTC;
  uint8_t  _pad6[0x80000 - (sizeof (struct intc_reg))];
  tmu_reg TMU;
  uint8_t  _pad7[0x80000 - (sizeof (struct tmu_reg))];
  sci_reg SCI;
  uint8_t  _pad8[0x80000 - (sizeof (struct sci_reg))];
  scif_reg SCIF;
  uint8_t  _pad9[0x80000 - (sizeof (struct scif_reg))];
  udi_reg UDI;
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

extern sh7091_reg SH7091;

