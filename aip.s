	.section .text.aip
	.global _text_aip_start

_text_aip_start:
	bra	label_10
	nop

label_4:
	.long 	0xac010000
label_8:
	.long 	0x00000000
label_c:
	.long	0xac00e100

label_10:
	mov.l	label_1c,r0	/* ac00e020 */
	jsr	@r0
	nop
label_16:
	bra	label_16
	nop
	nop
label_1c:
	.long	0xac00e020

/* called from label_10 */
label_20:
	mov.l	label_40,r2	/* ac00e0dc */
	mov.l	label_3c,r3	/* a0000000 */
	or	r3,r2
	jsr	@r2             /* jump to label_dc */
	nop
	stc	sr,r0
	mov.w	label_38,r1	/* ff0f */
	and	r1,r0
	or	#0xf0,r0        /* set IMASK ; disables interrupts */
	ldc	r0,sr
	bra	label_44
	nop
label_38:
	.word	0xff0f
label_3a:
	.word	0x0000
label_3c:
	.long	0xa0000000
label_40:
	.long	0xac00e0dc

label_44:
	mov.l	label_4c,r0	/* ac00f400 → new stack address */
	mov	r0,r15
	bra	label_50
	nop

label_4c:
	.long	0xac00f400

label_50:
	mov.l	label_70,r2	/* ac00e0d0 */
	mov.l	label_6c,r3	/* a0000000 */
	or	r3,r2
	jsr	@r2             /* jump to label_d0 */
	nop
label_5a:
	mov.l	label_78,r3	/* ac00e13c */
	mov.l	label_74,r4	/* ac00fc00 */
	mov.w	label_68,r6	/* 400 */
	jsr	@r3             /* jump to label_13c */
	mov	#0,r5
	bra	label_7c
	nop

label_68:
	.word	0x0400
label_6a:
	.word	0x0000
label_6c:
	.long	0xa0000000
label_70:
	.long	0xac00e0d0
label_74:
	.long	0xac00fc00
label_78:
	.long	0xac00e13c

label_7c:
	mov.l	label_c4,r0	/* 700000f0 */
	ldc	r0,sr
	sub	r0,r0
	mov	r0,r1
	mov	r0,r2
	mov	r0,r3
	mov	r0,r4
	mov	r0,r5
	mov	r0,r6
	mov	r0,r7
	mov	r0,r8
	mov	r0,r9
	mov	r0,r10
	mov	r0,r11
	mov	r0,r12
	mov	r0,r13
	mov	r0,r14
	mov.l	label_bc,r0	/* 8c00f400 */
	mov	r0,r15
	mov.l	label_c8,r0	/* 8c00f400 */
	ldc	r0,vbr
	mov.l	label_c0,r0	/* 40000 */
	lds	r0,fpscr
	mov.l	label_b8,r0	/* ac00e004 */
	mov.l	@r0,r0          /* ac010000 */
	jsr	@r0
	mov	r1,r0
label_b2:
	bra	label_b2
	nop
	nop

label_b8:
	.long	0xac00e004
label_bc:
	.long	0x8c00f400
label_c0:
	.long	0x00040000
label_c4:
	.long	0x700000f0
label_c8:
	.long	0x8c00f400

	/* unreachable label_cc ? */
label_cc:
	bra	label_cc
	nop
	/* called from label_50 */
label_d0:
	mov.l	label_d8,r2	/* ff00001c (CCN.CCR) */
	mov	#0,r3           /* disable all caches */
	rts
	mov.l	r3,@r2

label_d8:
	.long	0xff00001c

	/* called from label_20 */
label_dc:
	mov.l	label_f4,r0	/* ff00001c (CCN.CCR) */
	mov.l	@r0,r1
	mov.l	label_f8,r2	/* 89af → mask off undefined bits */
	and	r2,r1
	mov.w	label_f0,r2	/* 800 → ICI , instruction cache invalidation */
	or	r2,r1
	mov.l	r1,@r0
	bra	label_fc
	nop
	nop

label_f0:
	.word	 0x0800
	nop /* unreachable nop? */
label_f4:
	.long	0xff00001c
label_f8:
	.word	0x89af
label_fa:
	.word	0x0000

label_fc:
	rts
	nop

	mov.l	label_108,r0	/* 8c0000e0 */
	mov.l	@r0,r1
	jmp	@r1
	mov	#1,r4
label_108:
	.long	0x8c0000e0
label_10c:
	.long	0xac00e110

	.ascii "\nAIP Ver 1.07 Build:Apr 27 1999 14:00:30\n"
	.byte	0x00
	.byte 	0x00
	.byte 	0x00
	/* called from label_5a */
	/*
	memset(char * dest, char value, unsigned int length)
dest    :	r4
value   :	r5
length  :	r6
[return]: original value of `dest` as r0
	*/
label_13c:
	mov	#0,r7
	mov	r7,r3
	cmp/hs	r6,r3		/* r3 ≥ r6 (unsigned) */
				/* 0 ≥ 0x400 */
	bt/s    label_150
	mov	r4,r0           /* ac00fc00 → r0 */
label_146:
	add	#1,r7
	mov.b	r5,@r0          /* 0 → @(ac00fc00) */
	cmp/hs	r6,r7		/* r7 ≥ 0x400 */
	bf/s    label_146
	add	#1,r0
label_150:
	rts
	mov	r4,r0
