        .macro inner_multiplication

        /* y multiplication */
        mov #4,r1           /* r1 : temporary */
        fmov.s @r0,fr0      /* 0 */
        mov #8,r2           /* r2 : temporary */
        fmov.s @(r0,r1),fr1 /* 1 */
        fmov.s @(r0,r2),fr2 /* 2 */
        fldi0 fr3
        fipr fv8,fv0

        fmov.s @(r0,r5),fr4 /* r0 + 1280 */
        fmov.s @(r0,r6),fr5 /* r0 + 1281 */
        fmov.s @(r0,r7),fr6 /* r0 + 1282 */
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
        /* load fr1,fr5 */
        fmov.s @(r0,r3),fr1 /* r0 + 640 */
        /* exchange fr4/fr2 */
        fmov fr4,fr3
        fmov fr2,fr4
        fmov fr3,fr2
        fmov.s @(r0,r4),fr5 /* r0 + 642 */
        fldi0 fr3
        fipr fv8,fv0
        fldi0 fr7
        fipr fv12,fv4

        fadd fr3,fr7
        fmul fr7,fr7
        /* restore FPUL from y multiplication */
        fsts FPUL,fr3
        fadd fr3,fr7

        add #4,r0 /* next pixel */

        fschg
        fmov xd0,dr0 /* load 100.f constant */
        fcmp/gt fr0,fr7
        fschg

        .endm

        .macro sobel_fipr_inner_2px
        mov #0,r9

        inner_multiplication
        movt r9
        add #-1,r9
        extu.w r9,r9

        inner_multiplication
        movt r1
        add #-1,r1
        extu.w r1,r1
        shll16 r1
        or r1,r9
        .endm
