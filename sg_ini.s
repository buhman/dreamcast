	.section .text.sg_ini
	.global _text_sg_ini_start

_text_sg_ini_start:
	mov.l	label_18,r0	/* 8d000000 → beginning of system ram, image area */
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
	mov.l	label_204c,r0	/* 8c00d940 → label_2140 */
	mov	#0,r1
	jsr	@r0
	nop
	mov.l	label_2050,r0	/* 8c00d900 → label_2100 */
	jsr	@r0
	nop
	mov.l	label_2054,r0	/* 8c00d888 → label_2088 */
	jsr	@r0
	nop
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
	mov.l	@r0,r0
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

label_20ac:
	mov.l	label_20b4,r0	/* 8c0000e0 */
	mov.l	@r0,r0
	jmp	@r0
	mov	#1,r4
label_20b4:
	.long	0x8c0000e0

label_20b8:
	exts.b	r4,r0
	mov	#57,r1
	cmp/gt	r1,r0
	bf	label_20c2
	add	#-7,r0
label_20c2:
	rts
	add	#-48,r0
label_20c6:
	xor	r2,r2
	sts.l	pr,@-r15
	mov.b	@r4,r4
	mov	r4,r0
	cmp/eq	#32,r0
	bt	label_20e0
	bsr	label_20b8
	nop
	mov	#1,r1
	tst	r1,r0
	bt.s	label_20e0
	xor	r2,r2
	mov	#1,r2
label_20e0:
	lds.l	@r15+,pr
	rts
	mov	r2,r0
	.word 0xffff
label_20e8:
	mov.l	label_20fc,r1	! 8c008024
	mov.l	label_20f8,r3	! 4c494d20
	mov.l	@r1,r2
	cmp/eq	r3,r2
	movt	r0
	rts
	nop
	nop
label_20f8:
	.long 	0x4c494d20
label_20fc:
	.long	0x8c008024
label_2100:
	sts.l	pr,@-r15
	mov.l	label_212c,r3	/* 8c00d8c6 → label_20c6 */
	mov.l	label_213c,r4	! 8c008000
	jsr	@r3
	add	#62,r4
	tst	r0,r0
	bf	label_2126
	mov.l	label_2130,r3	/* 8c00d8e8 → label_20e8 */
	jsr	@r3
	nop
	tst	r0,r0
	bf	label_2126
	mov.l	label_2138,r3	! 8c010000
	mov.w	@r3,r0
	cmp/eq	#34,r0
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

label_2140:
	mov.l	r2,@-r15
	mov.l	r3,@-r15
	mov.l	r4,@-r15
	mov.l	r5,@-r15
	sts.l	pr,@-r15
	mova	label_2284,r0
	mov.w	@(r0,r1),r1
	add	r1,r0
label_2150:
	add	#3,r0
	shlr2	r0
	shll2	r0
	mov.w	@r0+,r4
	tst	r4,r4
	bf	label_2172
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
	mov.w	@r0+,r3
	mov.l	label_2278,r1	! ffff8000
	and	r3,r1
	tst	r1,r1
	bt	label_2180
	xor	r1,r3
	bf	label_2182
label_2180:
	mov.l	@r0+,r2
label_2182:
	mov.l	label_227c,r1	! 6000
	and	r3,r1
	tst	r1,r1
	bt	label_21a0
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
label_21a0:
	bsrf	r3
	dt	r4
	bf.s	label_21a0
	add	#4,r2
	bt	label_2150
	mov	#0,r1
	rts
	mov.l	r1,@r2
	mov	#0,r1
	rts
	mov.w	r1,@r2
	mov	#0,r1
	rts
	mov.b	r1,@r2
	mov.l	@r0+,r1
	rts
	mov.l	r1,@r2
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

	/* unreachable? */
	.word 0x8040
	.word 0xa05f
label_2284:
	.word 0x0002
	.word 0x0009
	.word 0x0002
	.word 0x0006
	.word 0x0038
	.word 0xff00
	.word 0x0001
	.word 0x0018
	.word 0x002c
	.word 0xffa0
	.word 0x12c0
	.word 0x0000
	.word 0x0004
	.word 0x000c
	.word 0x0000
	.word 0xffd0
	.word 0x0001
	.word 0x0018
	.word 0x0024
	.word 0xff00
	.word 0x0020
	.word 0x0000
	.word 0x0002
	.word 0x001e
	.word 0x7490
	.word 0xa05f
	.word 0x0222
	.word 0x0222
	.word 0x0002
	.word 0x001e
	.word 0x74a0
	.word 0xa05f
	.word 0x2001
	.word 0x2001
	.word 0x0002
	.word 0x001e
	.word 0x7890
	.word 0xa05f
	.word 0x001b
	.word 0x0271
	.word 0x0000
	.word 0x0000
	nop
	nop
	nop
	nop

label_22e0:
	sts.l	pr,@-r15
	mov.l	label_2330,r4	! 8c008000
	mov.l	label_2328,r3	! 8c00d8c6
	jsr	@r3
	add	#62,r4
	tst	r0,r0
	bt	label_2320
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
	mov.l	r3,@(16,r15)
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
	mov.l	@r0,r0
label_23e8:
	jmp	@r0
	nop
label_23ec:
	mov	#0,r6
	mov	#1,r7
	mov.l	label_2404,r0	! 8c0000bc
	mov.l	@r0,r0
	jmp	@r0
	nop
label_23f8:
	mov	#0,r6
	mov	#2,r7
	mov.l	label_2404,r0	! 8c0000bc
	mov.l	@r0,r0
	jmp	@r0
	nop
label_2404:
	.long	0x8c0000bc
