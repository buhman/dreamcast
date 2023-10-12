        .section .text.start
        .global _start
_start:
        /* set stack pointer */
        mov.l p1ram_end_ptr,r15

        /* mask all interrupts */
        mov.l   imask_all,r0
        stc     sr,r1
        or      r1,r0
        ldc     r0,sr

        /* jump to main */
        mov.l main_ptr,r0
        jmp @r0
        nop

        .align 4
p1ram_end_ptr:
        .long __p1ram_end
imask_all:
        .long 0xf0
main_ptr:
        .long _main
