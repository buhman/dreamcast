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
#define SCFSR2__TFDE (1 << 5) /* transmit fifo data empty */
#define SCFSR2__BRK  (1 << 4) /* break detect */
#define SCFSR2__FER  (1 << 3) /* framing error */
#define SCFSR2__PER  (1 << 2) /* parity error */
#define SCFSR2__RDF  (1 << 1) /* receive FIFO data full */
#define SCFSR2__DR   (1 << 0) /* receive data ready */
