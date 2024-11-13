	.global _illslot
_illslot:
        trapa #12
	rts
	nop
        mova	test,r0

test:	.long 0x12345678
