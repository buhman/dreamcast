        /* fv0 fv4 fv8 fv12 */
        .global _sobel_fipr
_sobel_fipr:
__setup:
        mov.l r8,@-r15
        mov.l r9,@-r15
        mov.l r10,@-r15
        mov.l r11,@-r15

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
        mova _const_100f,r0 /* r11 as temporary */
        fmov.s @r0,fr0
        fmov dr0,xd0

        /* save C arguments */
        mov r4,r0 /* r4 saved as r0 */
        mov r5,r8 /* r5 saved as r8 */

        /* offsets */
        mov #(1 * 4),r1
        mov #(2 * 4),r2
        mov.w _const_640,r3
        mov.w _const_642,r4
        mov.w _const_1280,r5
        mov.w _const_1281,r6
        mov.w _const_1282,r7

        add r3,r0 /* skip first row */
        add r3,r8
        add #4,r0 /* skip first pixel */
        add #4,r8
        mov.w _const_638,r10 /* skip last pixel */

        mov.w _const_478,r11 /* row count */

        bra _loop
        nop

        .align 4
_const_100f:    .float 100
_const_640:     .short (640 * 4)
_const_642:     .short (642 * 4)
_const_1280:    .short (1280 * 4)
_const_1281:    .short (1281 * 4)
_const_1282:    .short (1282 * 4)

_const_638:     .short 638
_const_478:     .short 478

        .align 4
_loop:

_loop_width:
        /* y multiplication */
        fmov.s @r0,fr0      /* 0 */
        fmov.s @(r0,r1),fr1 /* 1 */
        fmov.s @(r0,r2),fr2 /* 2 */
        fldi0 fr3
        fipr fv8,fv0

        fmov.s @(r0,r5),fr4 /* 1280 */
        fmov.s @(r0,r6),fr5 /* 1281 */
        fmov.s @(r0,r7),fr6 /* 1282 */
        fldi0 fr7
        fipr fv12,fv4

        fadd fr3,fr7
        fmul fr7,fr7

        /* save fr7 in FPUL */
        flds fr7,FPUL

        /* x multiplication */
        /* transpose and load
        before →
                fr0, fr1, fr2,   _,
                   ,    ,    ,    ,
                fr4, fr5, fr6,   _,

        after →
                fr0,    ,  fr4,  _,
                fr1,    ,  fr5,  _,
                fr2,    ,  fr6,  _,
        */
        /* exchange fr4/fr2 */
        fmov fr4,fr3
        fmov fr2,fr4
        fmov fr3,fr2
        /* load fr1,fr5 */
        fmov.s @(r0,r3),fr1 /* 640 */
        fldi0 fr3
        fipr fv8,fv0
        fmov.s @(r0,r4),fr5 /* 642 */
        fldi0 fr7
        fipr fv12,fv4

        fadd fr3,fr7
        fmul fr7,fr7
        /* restore FPUL from y multiplication */
        fsts FPUL,fr3
        fadd fr3,fr7

        fmov dr0,xd0 /* load 100.f constant */

        add #4,r0 /* next pixel */

        fcmp/gt fr0,fr7
        /*subc r9,r9*/
        movt r9
        add #-1,r9
        mov.l r9,@r8 /* save result */

        dt r10
        bf/s _loop_width
        add #4,r8
/* end of _loop_width */

        /* skip last pixel and first pixel */
        add #8,r8
        add #8,r0

        /* row decrement */
        dt r11
        mov.w _const_638_b,r10
        bf/s _loop
        nop

        /* restore registers */
_return:
        mov.l @r15+,r11
        mov.l @r15+,r10
        mov.l @r15+,r9
        mov.l @r15+,r8

        rts
        nop

_const_638_b:     .short 638
