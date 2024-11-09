        .section .text.start
        .global _start
_start:
        /* set stack pointer */
        mov.l stack_end_ptr,r15

        /* mask all interrupts */
        mov.l   imask_all,r0
        mov.l   zero_rb,r2
        stc     sr,r1
        or      r1,r0
        and     r0,r2
        ldc     r2,sr

	/* save pr */
	sts.l pr,@-r15

        /* jump to runtime_init */
        mov.l runtime_init_ptr,r0
        jsr @r0
        nop

	/* restore pr */
	lds.l @r15+,pr

	/* jump to main */
	mov.l main_ptr,r0
	jmp @r0
	nop

        .align 4
stack_end_ptr:
        .long __stack_end
imask_all:
        .long 0xf0
zero_rb:
        .long ~(1 << 29)
runtime_init_ptr:
        .long _runtime_init
main_ptr:
	.long _main
