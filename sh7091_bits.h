#define CCR__IIX (1 << 15)
#define CCR__ICI (1 << 11)
#define CCR__ICE (1 << 8)
#define CCR__OIX (1 << 7)
#define CCR__ORA (1 << 5)
#define CCR__OCI (1 << 4)
#define CCR__CB  (1 << 2)
#define CCR__WT  (1 << 1)
#define CCR__OCE (1 << 0)

#define PDTRA__MASK     (0b11 << 8)
#define PDTRA__VGA      (0b00 << 8)
#define PDTRA__RESERVED (0b01 << 8)
#define PDTRA__RGB      (0b10 << 8)
#define PDTRA__AV       (0b11 << 8)

#define SCFCR2__TFRST (1 << 2)
#define SCFCR2__RFRST (1 << 1)

#define SCSCR2__TE (1 << 5)
#define SCSCR2__RE (1 << 4)

#define SCFSR2__ER   (1 << 7) /* read error */
#define SCFSR2__TEND (1 << 6) /* transmit end */
#define SCFSR2__TDFE (1 << 5) /* transmit fifo data empty */
#define SCFSR2__BRK  (1 << 4) /* break detect */
#define SCFSR2__FER  (1 << 3) /* framing error */
#define SCFSR2__PER  (1 << 2) /* parity error */
#define SCFSR2__RDF  (1 << 1) /* receive FIFO data full */
#define SCFSR2__DR   (1 << 0) /* receive data ready */

#define DMAOR__DDT (1 << 15) /* on-demand data transfer mode */
/* priority mode */
#define DMAOR__PR__CH0_CH1_CH2_CH3 (0b11 << 8)
#define DMAOR__PR__CH0_CH2_CH3_CH1 (0b01 << 8)
#define DMAOR__PR__CH2_CH0_CH1_CH3 (0b10 << 8)
#define DMAOR__PR__ROUND_ROBIN     (0b11 << 8)
#define DMAOR__AE (1 << 2)  /* address error flag; clear-only */
#define DMAOR__NMIF (1 << 1) /* non-maskable interrupt flag; clear-only */
#define DMAOR__DME (1 << 0) /* DMAC master enable */

/* source address space attribute specification */
#define CHCR2__SSA__RESERVED_IN_PCMCIA_ACCESS    (0b000 << 29)
#define CHCR2__SSA__DYNAMIC_BUS_SIZING_IO_SPACE  (0b001 << 29)
#define CHCR2__SSA__8BIT_IO_SPACE                (0b010 << 29)
#define CHCR2__SSA__16BIT_IO_SPACE               (0b011 << 29)
#define CHCR2__SSA__8BIT_COMMON_MEMORY_SPACE     (0b100 << 29)
#define CHCR2__SSA__16BIT_COMMON_MEMORY_SPACE    (0b101 << 29)
#define CHCR2__SSA__8BIT_ATTRIBUTE_MEMORY_SPACE  (0b110 << 29)
#define CHCR2__SSA__16BIT_ATTRIBUTE_MEMORY_SPACE (0b111 << 29)
/* source address wait control select */
#define CHCR2__STC__C5_SPACE_WAIT_CYCLE_SELECTION (0 << 28)
#define CHCR2__STC__C6_SPACE_WAIT_CYCLE_SELECTION (1 << 28)
/* destination address space attribute specification */
#define CHCR2__DSA__RESERVED_IN_PCMCIA_ACCESS    (0b000 << 25)
#define CHCR2__DSA__DYNAMIC_BUS_SIZING_IO_SPACE  (0b001 << 25)
#define CHCR2__DSA__8BIT_IO_SPACE                (0b010 << 25)
#define CHCR2__DSA__16BIT_IO_SPACE               (0b011 << 25)
#define CHCR2__DSA__8BIT_COMMON_MEMORY_SPACE     (0b100 << 25)
#define CHCR2__DSA__16BIT_COMMON_MEMORY_SPACE    (0b101 << 25)
#define CHCR2__DSA__8BIT_ATTRIBUTE_MEMORY_SPACE  (0b110 << 25)
#define CHCR2__DSA__16BIT_ATTRIBUTE_MEMORY_SPACE (0b111 << 25)
/* destination address wait control select */
#define CHCR2__DTC__C5_SPACE_WAIT_CYCLE_SELECTION (0 << 24)
#define CHCR2__DTC__C6_SPACE_WAIT_CYCLE_SELECTION (1 << 24)
/* DREQ select */
#define CHCR2__DS__LOW_LEVEL_DETECTION    (0 << 19)
#define CHCR2__DS__FALLING_EDGE_DETECTION (1 << 19)
/* request check level */
#define CHCR2__RL__DRAK_IS_AN_ACTIVE_HIGH_OUTPUT (0 << 18)
#define CHCR2__RL__DRAK_IS_AN_ACTIVE_LOW_OUTPUT  (1 << 18)
/* acknowledge mode */
#define CHCR2__AM__DACK_IS_OUTPUT_IN_READ_CYCLE  (0 << 17)
#define CHCR2__AM__DACK_IS_OUTPUT_IN_WRITE_CYCLE (1 << 17)
/* acknowledge level */
#define CHCR2__AL__ACTIVE_HIGH_OUTPUT (0 << 16)
#define CHCR2__AL__ACTIVE_LOW_OUTPUT  (1 << 16)
/* destination address mode */
#define CHCR2__DM__DESTINATION_ADDRESS_FIXED       (0b00 << 14)
#define CHCR2__DM__DESTINATION_ADDRESS_INCREMENTED (0b01 << 14)
#define CHCR2__DM__DESTINATION_ADDRESS_DECREMENTED (0b10 << 14)
/* source address mode */
#define CHCR2__SM__SOURCE_ADDRESS_FIXED       (0b00 << 12)
#define CHCR2__SM__SOURCE_ADDRESS_INCREMENTED (0b01 << 12)
#define CHCR2__SM__SOURCE_ADDRESS_DECREMENTED (0b10 << 12)
/* resource select */
#define CHCR2__RS(n) (((n) & 0b1111) << 8)
/* transmit mode */
#define CHCR2__TM__CYCLE_STEAL_MODE (0 << 7)
#define CHCR2__TM__BURST_MODE       (1 << 7)
/* transmit size */
#define CHCR2__TS__64_BIT  (0b000 << 4)
#define CHCR2__TS__8_BIT   (0b001 << 4)
#define CHCR2__TS__16_BIT  (0b010 << 4)
#define CHCR2__TS__32_BIT  (0b011 << 4)
#define CHCR2__TS__32_BYTE (0b100 << 4)
#define CHCR2__IE  (1 << 2) /* interrupt enable */
#define CHCR2__TE  (1 << 1) /* transfer end; clear only */
#define CHCR2__DE  (1 << 0) /* DMAC (channel) enable */

#define DMATCR2__TRANSFER_COUNT(n) (((n) & 0xffffff) << 0)
