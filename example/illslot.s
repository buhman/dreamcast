	.global _illslot
_illslot:
	rts
	mova	test,r0

test:	.long 0x12345678
