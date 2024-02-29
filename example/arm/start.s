        .macro FILL_ZERO_32_BYTE_ALIGNED
        cmp r1, r10
        beq _fill_break.\@
        .irp i, 2, 3, 4, 5, 6, 7, 8, 9
        mov r\i, #0
        .endr
_fill_loop.\@:
        stmia r1!, {r2 - r9}
        cmp r1, r10
        blt _fill_loop.\@
_fill_break.\@:
        .endm

        .section .text.start
        .global _start
_start:
	b _reset
_reset:
_link_bss:
	/*
        ldr r1, =__bss_link_start
        ldr r10, =__bss_link_end
        FILL_ZERO_32_BYTE_ALIGNED
	*/

	/* set stack pointer */
        ldr sp,=__ram_end

        bl main
