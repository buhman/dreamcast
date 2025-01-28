        ocbi @r13
        pref @r13 /* 32 bytes, 8 pixels */

        /* unpack the next 8 pixels */

        fschg

        .include "unpack_pixel_inner.s"
        float fpul,fr0
        .include "unpack_pixel_inner.s"
        float fpul,fr1
        .include "unpack_pixel_inner.s"
        float fpul,fr2
        .include "unpack_pixel_inner.s"
        float fpul,fr3
        .include "unpack_pixel_inner.s"
        float fpul,fr4
        .include "unpack_pixel_inner.s"
        float fpul,fr5
        .include "unpack_pixel_inner.s"
        float fpul,fr6
        .include "unpack_pixel_inner.s"
        float fpul,fr7

        fmov dr0,@r12
        add #8,r12
        fmov dr2,@r12
        add #8,r12
        fmov dr4,@r12
        add #8,r12
        fmov dr6,@r12
        add #8,r12

        fschg
