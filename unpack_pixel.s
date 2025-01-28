        /* unpack the next 8 pixels */

        /*
        mov.l @r13,r9
        extu.b r9,r1
        shlr8 r9
        extu.b r9,r2
        add r1,r2
        shlr8 r9
        extu.b r9,r1
        add r1,r2
        shlr8 r9
        add r2,r9
        lds r9,fpul
        add #4,r13
        */
        .macro unpack_pixel_inner_nibs

        mov.w @r13+,r9

        mov r9,r1 /* nib0 */
        shlr2 r9
        shlr2 r9
        and r14,r1

        mov r9,r2 /* nib1 */
        shlr2 r9
        shlr2 r9
        and r14,r2
        add r2,r1

        mov r9,r2  /* nib3 */
        shlr2 r9
        shlr2 r9
        and r14,r2
        add r2,r1

        and r14,r9 /* nib4 */
        add r9,r1

        lds r1,fpul

        .endm

        .macro unpack_pixel_8
        unpack_pixel_inner_nibs
        float fpul,fr0
        unpack_pixel_inner_nibs
        float fpul,fr1
        unpack_pixel_inner_nibs
        float fpul,fr2
        unpack_pixel_inner_nibs
        float fpul,fr3
        unpack_pixel_inner_nibs
        float fpul,fr4
        unpack_pixel_inner_nibs
        float fpul,fr5
        unpack_pixel_inner_nibs
        float fpul,fr6
        unpack_pixel_inner_nibs
        float fpul,fr7

        fmov dr0,@r12
        add #8,r12
        fmov dr2,@r12
        add #8,r12
        fmov dr4,@r12
        add #8,r12
        fmov dr6,@r12
        add #8,r12
        .endm

        .macro unpack_pixel_16
        ocbi @r13
        pref @r13 /* 32 bytes, 16 pixels */
        mov #15,r14

        fschg

        unpack_pixel_8
        unpack_pixel_8

        fschg
        .endm
