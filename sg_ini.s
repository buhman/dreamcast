	.section .text.sg_ini
	.global _text_sg_ini_start

_text_sg_ini_start:
	mov.l	label_18,r0	/* 8d000000 → end of system ram */
	mov	r0,r15
	nop
	nop
	mov.l	label_1c,r0	/* 8c00d820 → label_2020 */
	jmp	@r0
	nop
	nop
	nop
	nop
	nop
	nop

label_18:
	.long	0x8d000000
label_1c:
	.long	0x8c00d820
label_20:
	.fill	0x2000,1,0x00

label_2020:
	/* function that processes the list of addresses/values at label_2284 */
	mov.l	label_204c,r0	/* 8c00d940 → label_2140 */
	mov	#0,r1
	jsr	@r0
	nop

	/* ???? possibly jump to boot menu? */
	mov.l	label_2050,r0	/* 8c00d900 → label_2100 */
	jsr	@r0
	nop

	/* possibly jump to boot menu? */
	mov.l	label_2054,r0	/* 8c00d888 → label_2088 */
	jsr	@r0
	nop

	/* check for Windows CE? */
	mov.l	label_2058,r0	/* 8c00dae0 → label_22e0 */
	jsr	@r0
	nop

	mov.l	label_205c,r0	/* 8c00db40 → label_2340 */
	jsr	@r0
	nop

	nop
	mov.l	label_2060,r0	/* 8c00d86c → label_206c */
	jmp	@r0
	nop

	nop
	nop

label_204c:
	.long	0x8c00d940
label_2050:
	.long	0x8c00d900
label_2054:
	.long	0x8c00d888
label_2058:
	.long	0x8c00dae0
label_205c:
	.long	0x8c00db40
label_2060:
	.long	0x8c00d86c

	.ascii "INI90405"

label_206c:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	mov.l	label_2084,r0	/* 8c00e000 */
	jmp	@r0
	nop
label_2084:
	.long	0x8c00e000	/* aip */

label_2088:
	sts.l	pr,@-r15
	mov.l	label_20a4,r0	/* 8c000048 */
	mov.l	@r0,r0		/* r0 ← 1 */
	cmp/eq	#6,r0
	bt	label_209e
	cmp/eq	#7,r0
	bt	label_209e
	cmp/eq	#9,r0
	bt	label_209e
	rts
	lds.l	@r15+,pr
label_209e:
	mov.l	label_20a8,r1	/* 8c00d8ac → label_20ac */
	jmp	@r1
	nop
label_20a4:
	.long	0x8c000048
label_20a8:
	.long	0x8c00d8ac

	/* jump back to boot menu? */
label_20ac:
	mov.l	label_20b4,r0	/* 8c0000e0 */
	mov.l	@r0,r0
	jmp	@r0
	mov	#1,r4
label_20b4:
	.long	0x8c0000e0

label_20b8:
	exts.b	r4,r0
	mov	#57,r1		/* ascii '9' */
	cmp/gt	r1,r0
	bf	label_20c2
	add	#-7,r0
label_20c2:
	rts
	add	#-48,r0

	/* called from label_2100 and label_22e0 */
label_20c6:
	xor	r2,r2
	sts.l	pr,@-r15
	mov.b	@r4,r4		/* r4 ← (0x8c00803e) = 0x30 */
	mov	r4,r0		/* r0 ← 0x30 */
	cmp/eq	#0x20,r0	/* check for ascii space in 'Compatible peripherals' of systemid */
	bt	label_20e0	/* if it is space, return 0 */
	bsr	label_20b8	/* ascii 0-9 to integer */
	nop
	mov	#1,r1
	tst	r1,r0		/* check for ascii '1' / "uses Windows CE" */
	bt.s	label_20e0	/* if not "uses Windows CE", take branch to label_20e0 */
	xor	r2,r2
	mov	#1,r2
label_20e0:
	lds.l	@r15+,pr
	rts
	mov	r2,r0		/* return 1 if Windows CE, 0 if not */
	.word 0xffff
label_20e8:
	mov.l	label_20fc,r1	! 8c008024
	mov.l	label_20f8,r3	/* ascii ' MIL' ; 0x4c494d20 */
	mov.l	@r1,r2		/* r2 ← 0x2d444720 ' GD-' */
	cmp/eq	r3,r2
	movt	r0
	rts
	nop
	nop
label_20f8:
	.ascii	" MIL"	/* 4c494d20 */
label_20fc:
	.long	0x8c008024

	/* called from label_2020 */
label_2100:
	sts.l	pr,@-r15
	mov.l	label_212c,r3	/* 8c00d8c6 → label_20c6 */
	mov.l	label_213c,r4	/* 8c008000 */
	jsr	@r3
	add	#0x3e,r4	/* r4 ← 0x8c00803e */
	tst	r0,r0		/* check return value != 0 */
	bf	label_2126

	/* check for ' MIL' CD */
	mov.l	label_2130,r3	/* 8c00d8e8 → label_20e8 */
	jsr	@r3
	nop
	tst	r0,r0		/* check return value != 0 */
	bf	label_2126

	mov.l	label_2138,r3	! 8c010000
	mov.w	@r3,r0
	cmp/eq	#34,r0		/* check for 'stc vbr,r0' ?? */
	bf	label_2126
	mov.l	label_2134,r1	/* 8c00d8ac → label_20ac */
	jmp	@r1
	lds.l	@r15+,pr
label_2126:
	lds.l	@r15+,pr
	rts
	nop
label_212c:
	.long	0x8c00d8c6
label_2130:
	.long	0x8c00d8e8
label_2134:
	.long	0x8c00d8ac
label_2138:
	.long	0x8c010000
label_213c:
	.long	0x8c008000

	/* function that processes the list of addresses/values at label_2284 */
label_2140:
	mov.l	r2,@-r15
	mov.l	r3,@-r15
	mov.l	r4,@-r15
	mov.l	r5,@-r15
	sts.l	pr,@-r15
	mova	label_2284,r0	/* 0x8c00da84 */
	mov.w	@(r0,r1),r1	/* 0x2 */
	add	r1,r0		/* 0x8c00da86 */
label_2150:
	add	#3,r0
	shlr2	r0
	shll2	r0		/* #1 r0 ← 0x8c00da88
				   #2 r0 ← 0x8c00da90 */
	mov.w	@r0+,r4		/* #1 r0 ← 0x8c00da8a ; r4 ← 2
				   #2 r4 ← 1 */
	tst	r4,r4		/* r4 & r4 != 0 → T unset */
	bf	label_2172	/* branch taken */
	mov.w	@r0+,r4
	tst	r4,r4
	bf.s	label_2150
	add	r4,r0
	lds.l	@r15+,pr
	mov.l	@r15+,r5
	mov.l	@r15+,r4
	mov.l	@r15+,r3
	mov.l	@r15+,r2
	rts
	sett
label_2172:
	mov.w	@r0+,r3		/* #1 0x8c00da8a ; r0 ← 0x8c00da8c ; r3 ← 0x6
				   #2 0x8c00da92 ; r0 ← 0x8c00da94 ; r3 ← 0x18
				*/
	mov.l	label_2278,r1	/* ffff8000 */
	and	r3,r1		/* r1 ← 0 */
	tst	r1,r1		/* r1 & r1 == 0 → T set */
	bt	label_2180	/* branch taken */
	xor	r1,r3
	bf	label_2182
label_2180:
	mov.l	@r0+,r2		/* 0x8c00da8c ; r2 ← 0xff000038 */
label_2182:
	mov.l	label_227c,r1	/* r1 ← 6000 */
	and	r3,r1		/* r1 ← 0 */
	tst	r1,r1		/* T set */
	bt	label_21a0	/* branch taken */
	xor	r1,r3
	add	#12,r3
	mov.l	@r0+,r1
	mov	r0,r5
	add	r1,r0
label_2194:
	bsrf	r3
	dt	r4
	bf.s	label_2194
	add	#4,r2
	bra	label_2150
	mov	r5,r0
	/* branch from label_2182 */
label_21a0:
	bsrf	r3		/* #1 0x06 ; PC ← 0x8c00d9aa (label_21aa)
				   #2 0x18 ; PC ← 0x8c00d9bc (label_2abc)
				   #3 0x0c ; PC ← 0x8c00d9b0 (label_21b0)
	                        */
	dt	r4		/* #1 r4 ← 1 ; T unset
				   #2 r4 ← 0 ; T set
				   #3 r4 ← 3 ; T unset
	                        */
	bf.s	label_21a0 	/* #1 branch taken ; #2 branch not taken */
	add	#4,r2		/* 0xff000038 ; r2 ← 0xff00003c */
	bt	label_2150	/* branch taken */
label_21aa:
	mov	#0,r1
	rts
	mov.l	r1,@r2
label_21b0:
	mov	#0,r1
	rts
	mov.w	r1,@r2
	mov	#0,r1
	rts
	mov.b	r1,@r2
label_21bc:
	mov.l	@r0+,r1		/* 0x8c00da98 ; r1 ← 0x12c0 */
	rts
	mov.l	r1,@r2		/* 0xffa0002c ; CHCR2 ← 0x000012c0
				DM: fixed
				SM: incremented
				RS: external request, single address mode
				*/
label_21c2:
	mov.w	@r0+,r1
	rts
	mov.l	r1,@r2

	mov.l	@r0+,r1
	rts
	mov.b	r1,@r2
	mov	#-1,r1
	rts
	mov.l	r1,@r2
	mov.w	@r0+,r1
	rts
	mov.w	r1,@r2
	mov.l	@r0+,r1
	rts
	mov.w	r1,@r2
	nop
	rts
	mov.l	@r2,r1
	bra	label_2228
	nop
	nop
	xor	r1,r1
	sts.l	pr,@-r15
	mov.l	r3,@-r15
label_21f2:
	mov	#8,r3
label_21f4:
	mov.l	r1,@r2
	dt	r3
	bf.s	label_21f4
	add	#4,r2
	bsr	label_2252
	nop
	dt	r4
	bf	label_21f2
	add	#1,r4
	add	#-4,r2
	mov.l	@r15+,r3
	lds.l	@r15+,pr
	rts
	sett
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
label_2228:
	sts.l	pr,@-r15
	mov.l	r0,@-r15
	mov.l	r1,@-r15
	mov.l	r2,@-r15
	mov.l	r3,@-r15
	mov.l	r4,@-r15
	mov.l	r5,@-r15
	mov.l	r6,@-r15
	jsr	@r2
	mov.l	r7,@-r15
	mov.l	@r15+,r7
	mov.l	@r15+,r6
	mov.l	@r15+,r5
	mov.l	@r15+,r4
	mov.l	@r15+,r3
	mov.l	@r15+,r2
	mov.l	@r15+,r1
	mov.l	@r15+,r0
	lds.l	@r15+,pr
	rts
	sett
label_2252:
	mov.l	r0,@-r15
	mov.l	r1,@-r15
	mov.l	r2,@-r15
	mov.l	label_2270,r1	! a05f688c
	mov.l	label_2274,r2	! 1800
label_225c:
	dt	r2
	bt	label_2266
	mov.l	@r1,r0
	tst	#1,r0
	bf	label_225c
label_2266:
	mov.l	@r15+,r2
	mov.l	@r15+,r1
	rts
	mov.l	@r15+,r0
	nop
label_2270:
	.long	0xa05f688c
label_2274:
	.long	0x00001800

label_2278:
	.long	0xffff8000
label_227c:
	.long	0x00006000

	.long	0xa05f8040 /* unused; VO_BORDER_COL */
label_2284:
	.word 0x0002
label_2286:
	.word 0x0009	 /* unused garbage data? */

label_2288:
	.word 0x0002
label_228a:
	.word 0x0006	/* r3 */
label_228c:
	.long 0xff000038

label_2290:
	.word 0x0001
label_2292:
	.word 0x0018	/* r3 */
label_2294:
	.long 0xffa0002c /* CHCR2 ← 0x000012c0 */
label_2298:
	.long 0x000012c0

label_229c:
	.word 0x0004
label_229e:
	.word 0x000c
label_22a0:
	.long 0xffd00000 /* ICR ← 0
	                    IPRA ← 0
	                    IPRB ← 0
	                    IPRC ← 0
	                 */
label_22a4:
	.word 0x0001     /* r4 */
label_22a6:
	.word 0x0018     /* r3 → bsrf label_21bc */
label_22a8:
	.long 0xff000024 /* r2
			    ?? ← 0x00000020 */
label_22ac:
	.long 0x00000020 /* r1 */

label_22b0:
	.word 0x0002	 /* r4 */
label_22b2:
	.word 0x001e	 /* r3 → bsrf label_21c2 */
label_22b4:
	.long 0xa05f7490 /* r2
			    SB_G1CRC ← 0x0222
			    SB_G1CWC ← 0x0222
	                  */
label_22b8:
	.word 0x0222
label_22ba:
	.word 0x0222

label_22bc:
	.word 0x0002	 /* r4 */
label_22be:
	.word 0x001e	 /* r3 → bsrf label_21c2 */
label_22c0:
	.long 0xa05f74a0 /* r2
			    SB_G1GDRC ← 0x2001
			    SB_G1GDWC ← 0x2001
	*/
label_22c4:
	.word 0x2001
label_22c6:
	.word 0x2001

label_22c8:
	.word 0x0002	/* r4 */
label_22ca:
	.word 0x001e	/* r3 → bsrf label_21c2 */
label_22cc:
	.long 0xa05f7890 /* SB_G2DSTO ← 0x001b
			    SB_G2TRTO ← 0x0271
			*/
label_22d0:
	.word 0x001b
label_22d2:
	.word 0x0271

label_22d4:
	.word 0x0000
	.word 0x0000
	nop
	nop
	nop
	nop

	/* called from label_2020 */
label_22e0:
	sts.l	pr,@-r15
	/* check systemid for "uses Windows CE" */
	mov.l	label_2330,r4	/* 8c008000 systemid */
	mov.l	label_2328,r3	/* 8c00d8c6 label_20c6 */
	jsr	@r3
	add	#0x3e,r4
	tst	r0,r0		/* r0 (1 if Windows CE) */
	bt	label_2320	/* if not Windows CE, return */
	mov.l	label_2334,r1	! 8ce01010
	mov.l	@r1,r0
	tst	r0,r0
	bt	label_231a
	mov.l	label_233c,r2	! a8
	cmp/hi	r2,r0
	bt	label_231a
	dt	r0
	mov	r0,r2
	shll	r2
	add	r0,r2
	shll2	r2
	add	r2,r1
	add	#12,r1
	mov.l	@r1,r2
	add	#4,r1
	mov.l	@r1,r3
	add	r3,r2
	mov.l	label_2338,r0	! a05f74f4
	mov.l	@r0,r3
	cmp/eq	r3,r2
	bt	label_2320
label_231a:
	mov.l	label_232c,r1	! 8c00d8ac
	jmp	@r1
	lds.l	@r15+,pr
label_2320:
	lds.l	@r15+,pr
	rts
	nop
	nop
label_2328:
	.long	0x8c00d8c6
label_232c:
	.long	0x8c00d8ac
label_2330:
	.long	0x8c008000
label_2334:
	.long	0x8ce01010
label_2338:
	.long	0xa05f74f4
label_233c:
	.long	0x000000a8

label_2340:
	mov.l	r14,@-r15
	mov	#-1,r14
	sts.l	pr,@-r15
	add	#-20,r15
	mov	r15,r3
	mov	r15,r5
	mov.l	r3,@(16,r15)	/* copy stack pointer to stack */
	add	#16,r5
	bsr	label_23e0
	mov	#30,r4

	mov	r0,r4
	tst	r4,r4
	bt	label_2384
	bsr	label_2396
	nop
	tst	r0,r0
	bf	label_2384
	mov.l	@(4,r15),r4
	mov.l	label_23d4,r3	! ffff
	cmp/hi	r3,r4
	bt	label_2384
	mov	r15,r5
	mov.w	label_23d0,r1	! e10
	mov.l	r1,@(4,r15)
	bsr	label_23e0
	mov	#31,r4
	mov	r0,r4
	tst	r4,r4
	bt	label_2384
	bsr	label_2396
	nop
	tst	r0,r0
	bf	label_2384
	mov	#0,r14
label_2384:
	tst	r14,r14
	bt	label_238e
	mov.l	label_23d8,r2	! 8c00d8ac
	jsr	@r2
	nop
label_238e:
	add	#20,r15
	lds.l	@r15+,pr
	rts
	mov.l	@r15+,r14
label_2396:
	mov.l	r14,@-r15
	mov.l	r13,@-r15
	sts.l	pr,@-r15
	mov	#0,r14
	add	#-16,r15
	mov	r4,r13
label_23a2:
	bsr	label_23f8
	nop
	mov	r15,r5
	bsr	label_23ec
	mov	r13,r4
	cmp/eq	#-1,r0
	bt	label_23c4
	cmp/eq	#1,r0
	bt	label_23bc
	cmp/eq	#2,r0
	bf	label_23c4
	bra	label_23c6
	mov	#0,r0
label_23bc:
	mov.l	label_23dc,r4	! 10000
	add	#1,r14
	cmp/ge	r4,r14
	bf	label_23a2
label_23c4:
	mov	#-1,r0
label_23c6:
	add	#16,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r13
	rts
	mov.l	@r15+,r14
label_23d0:
	.word 	0x0e10
	.word	0xffff
label_23d4:
	.long	0x0000ffff
label_23d8:
	.long	0x8c00d8ac
label_23dc:
	.long	0x00010000

label_23e0:
	mov	#0,r6
	mov	#0,r7
	mov.l	label_2404,r0	! 8c0000bc
	mov.l	@r0,r0		/* r0 ← 0x8c001000 */
label_23e8:
	jmp	@r0
	nop
label_23ec:
	mov	#0,r6
	mov	#1,r7
	mov.l	label_2404,r0	! 8c0000bc
	mov.l	@r0,r0		/* r0 ← 0x8c001000 */
	jmp	@r0
	nop
label_23f8:
	mov	#0,r6
	mov	#2,r7
	mov.l	label_2404,r0	! 8c0000bc
	mov.l	@r0,r0		/* r0 ← 0x8c001000 */
	jmp	@r0
	nop
label_2404:
	.long	0x8c0000bc
