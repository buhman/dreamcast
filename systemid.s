	/*
<A><-------B------> <C->

1100 0000 0000 0000 1000 0001 0000

0000 0000 0000 0000 0000 0000 0000
^^^^ ^^^^ ^^^^ ^^^^ ^^^^    ^    ^
|||| |||| |||| |||| ||||    |    |
|||| |||| |||| |||| ||||    |    +----- Uses Windows CE
|||| |||| |||| |||| ||||    |
|||| |||| |||| |||| ||||    +-----  VGA box support
|||| |||| |||| |||| ||||
|||| |||| |||| |||| |||+----- Other expansions
|||| |||| |||| |||| ||+----- Puru Puru pack
|||| |||| |||| |||| |+----- Mike device
|||| |||| |||| |||| +----- Memory card
|||| |||| |||| |||+------ Start + A + B + Directions
|||| |||| |||| ||+------ C button
|||| |||| |||| |+------ D button
|||| |||| |||| +------ X button
|||| |||| |||+------- Y button
|||| |||| ||+------- Z button
|||| |||| |+------- Expanded direction buttons
|||| |||| +------- Analog R trigger
|||| |||+-------- Analog L trigger
|||| ||+-------- Analog horizontal controller
|||| |+-------- Analog vertical controller
|||| +-------- Expanded analog horizontal
|||+--------- Expanded analog vertical
||+--------- Gun
|+--------- Keyboard
+--------- Mouse
	*/

	.section .text.systemid

	.ascii "SEGA SEGAKATANA " /* H/W identifier */
	.ascii "SEGA ENTERPRISES" /* H/W Vendor ID */
	.ascii "39F1 "            /* Media ID */
	.ascii      "GD-ROM1/1  " /* Media information */
	.ascii "JUE     "         /* Compatible Area Symbol */
	.ascii         "C000810	" /* Compatible peripherals */
	.ascii "HDR-0900  "       /* Product number */
	.ascii           "V0.000" /* Version number */
	.ascii "19980901"         /* Release date */
	.ascii         "        " /* Reserved */
	.ascii "1ST_READ.BIN"
	.ascii             "    " /* Reserved */
	.ascii "SEGA LC-KAISYAID" /* Maker identifier */
	.ascii "SAMPLE GAME     " /* Game title */
	.fill  (96-16),1,0x20     /* Game title */
	.fill  32,1,0x20          /* Reserved */
