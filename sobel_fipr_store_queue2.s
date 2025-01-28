        /* fv0 fv4 fv8 fv12 */
        .global _sobel_fipr_store_queue2
_sobel_fipr_store_queue2:
        /* r0:  var   (buffer address) */
        /* r1:  (temporary)         */
        /* r2:  (temporary)         */
        /* r3:  const 640   4         */
        /* r4:  const 642   4         */
        /* r5:  const 1280  4         */
        /* r6:  const 1281  4         */
        /* r7:  const 1282  4         */
        /* r8:  var   (output address / store queue)                 */
        /* r9:  (temporary)                             */
        /* r10: var   (x loop counter)                               */
        /* r11: var   (y loop counter)                               */
        /* r12: var   (prefetch address: input address + 1280  4)    */
        /* r13: var   (input address) */
        /* r14: (temporary)    */

__setup:
        mov.l r8,@-r15
        mov.l r9,@-r15
        mov.l r10,@-r15
        mov.l r11,@-r15
        mov.l r12,@-r15
        mov.l r13,@-r15
        mov.l r14,@-r15
        fmov.s  fr12,@-r15
        fmov.s  fr13,@-r15
        fmov.s  fr14,@-r15
        fmov.s  fr15,@-r15

        fldi1 fr8    /* 1.0 */
        fldi1 fr9    /* 2.0 */
        fldi1 fr10   /* 1.0 */
        fldi0 fr11   /* 0.0 */
        fadd fr9,fr9

        fldi1 fr12
        fmov fr9,fr13
        fldi1 fr14
        fldi0 fr15
        fneg fr12
        fneg fr13
        fneg fr14

        /* constants */
        mova _const_100f,r0    /* use r0 as temporary */
        fmov.s @r0,fr0
        fschg
        fmov dr0,xd0
        fschg

        /* set qacr0 */
        mov r5,r0              /* r5: C argument */
        shlr16 r0              /* use r0 as temporary */
        mov.l _const_qacr0,r9  /* use r9 as temporary */
        shlr8 r0
        and #28,r0 /* 0b11100 */
        mov.l r0,@r9
        mov.l r0,@(4,r9) /* qacr1 */

        /* translate r8 to store queue address; keep bits [25:6] */
        mov r5,r8                        /* r5: C argument */
        mov.l _const_store_queue_mask,r0 /* use r0 as temporary */
        and r0,r8
        mov.l _const_store_queue,r9      /* use r9 as temporary */
        or r9,r8                         /* 0xe0000000 | (in_addr & 0x03ffffc0) */

        /* save C input argument */
        mov r4,r13 /* r4 saved as r13 (input data) */
        mov r6,r0  /* r6 saved as r0  (temporary buffer) */

        /* offsets */
        mov.w _const_640,r3
        mov.w _const_642,r4
        mov.w _const_1280,r5
        mov.w _const_1281,r6
        mov.w _const_1282,r7

        bra _prime_pixels_loop_init
        nop

        .align 4
_const_100f:    .float 50

_const_store_queue:             .long 0xe0000000
_const_store_queue_mask:        .long 0x03ffffc0 /* (0xffffffff & (~0b111111)) & (~(0b111111 << 26)) */
_const_qacr0:                   .long 0xff000038

_const_640:     .short (640 * 4)
_const_642:     .short (642 * 4)
_const_1280:    .short (1280 * 4)
_const_1281:    .short (1281 * 4)
_const_1282:    .short (1282 * 4)

        /* use r10 as temporary to load the first 1280 pixels; 16 pixels per loop iteration */
        .include "unpack_pixel.s"
        .align 4
_prime_pixels_loop_init:
        mov r0,r12
        mov #80,r10               /* 1280 / 16 */
        shll r10

_prime_pixels_loop:
        unpack_pixel_16
        dt r10
        bt _loop_init
        bra _prime_pixels_loop
        nop

_loop_init:
        /* skip first output row */
        mov r3,r1
        shlr r1
        add r1,r8 /* r3: 640 * 4 */

        mov.w _const_height,r11   /* 478      */
        bra _loop
        mov #40,r10               /* 640 / 8 */

_const_height:     .short 476
/*_const_height:     .short 238*/

        .include "sobel_fipr_inner2.s"
_loop:
_loop_width:
        /* prefetch at r8 + 1280 */
        unpack_pixel_16

        /* process the next 16 pixels */
        sobel_fipr_inner_2px
        mov.l r9,@r8     /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(4,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(8,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(12,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(16,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(20,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(24,r8) /* save result in the store queue */
        sobel_fipr_inner_2px
        mov.l r9,@(28,r8) /* save result in the store queue */

        /* send the store queue */
        pref @r8
        add #32,r8

        dt r10
        bt _row_decrement
        bra _loop_width
        nop
        /* end of _loop_width */

_row_decrement:
        /* row decrement */
        dt r11
        bt _return
        bra _loop
        mov #40,r10 /* 640 / 8 */

        /* restore registers */
_return:
        fmov.s  @r15+,fr15
        fmov.s  @r15+,fr14
        fmov.s  @r15+,fr13
        fmov.s  @r15+,fr12
        mov.l @r15+,r14
        mov.l @r15+,r13
        mov.l @r15+,r12
        mov.l @r15+,r11
        mov.l @r15+,r10
        mov.l @r15+,r9
        mov.l @r15+,r8

        rts
        nop

_const_638_b:     .short 638
