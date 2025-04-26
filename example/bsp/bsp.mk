BSP_20KDM2_OBJ = \
	example/bsp/20kdm2.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	interrupt.o \
	maple/maple.o \
	font/font_bitmap.o \
	font/verite_8x16/verite_8x16.data.o \
	bsp/20kdm2/maps/20kdm2.bsp.o \
	bsp/20kdm2/textures/e7/e7walldesign01b.data.o \
	bsp/20kdm2/textures/e7/e7steptop2.data.o \
	bsp/20kdm2/textures/e7/e7dimfloor.data.o \
	bsp/20kdm2/textures/e7/e7brickfloor01.data.o \
	bsp/20kdm2/textures/e7/e7bmtrim.data.o \
	bsp/20kdm2/textures/e7/e7sbrickfloor.data.o \
	bsp/20kdm2/textures/e7/e7brnmetal.data.o \
	bsp/20kdm2/textures/e7/e7beam02_red.data.o \
	bsp/20kdm2/textures/e7/e7swindow.data.o \
	bsp/20kdm2/textures/e7/e7bigwall.data.o \
	bsp/20kdm2/textures/e7/e7panelwood.data.o \
	bsp/20kdm2/textures/e7/e7beam01.data.o \
	bsp/20kdm2/textures/gothic_floor/xstepborder5.data.o \
	bsp/20kdm2/textures/liquids/lavahell.data.o \
	bsp/20kdm2/textures/e7/e7steptop.data.o \
	bsp/20kdm2/textures/gothic_trim/metalblackwave01.data.o \
	bsp/20kdm2/textures/stone/pjrock1.data.o \
	bsp/20kdm2/models/mapobjects/timlamp/timlamp.data.o \
	bsp/20kdm2/textures/sfx/flame2.data.o \
	bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2.data.o \
	bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2b.data.o \
	$(LIBGCC)

example/bsp/20kdm2.elf: LDSCRIPT = $(LIB)/main.lds
example/bsp/20kdm2.elf: $(START_OBJ) $(BSP_20KDM2_OBJ)

BSP_KATDM3_OBJ = \
	example/bsp/katdm3.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	interrupt.o \
	maple/maple.o \
	font/font_bitmap.o \
	font/verite_8x16/verite_8x16.data.o \
	bsp/katdm3/maps/katdm3.bsp.o \
	bsp/katdm3/textures/katdm3/kt_rivit-strap2_cont_dk.data.o \
	bsp/baseq3/textures/gothic_trim/metalsupport4small.data.o \
	bsp/katdm3/textures/katdm3/kt_rock_1f_shiny_dk.data.o \
	bsp/baseq3/textures/gothic_floor/largerblock3b3x128.data.o \
	bsp/katdm3/textures/katdm3/kt_rock_1f_rot_fade.data.o \
	bsp/baseq3/textures/gothic_trim/km_arena1tower4_a.data.o \
	bsp/katdm3/textures/katdm3/kattex-3-stone-bmp.data.o \
	bsp/baseq3/textures/gothic_block/smblk3b3dim_wall.data.o \
	bsp/baseq3/textures/gothic_trim/border2.data.o \
	bsp/baseq3/textures/gothic_wall/streetbricks10.data.o \
	bsp/baseq3/textures/gothic_block/blocks18b.data.o \
	bsp/baseq3/textures/gothic_trim/metalsupsolid.data.o \
	bsp/baseq3/textures/gothic_trim/km_arena1tower4.data.o \
	bsp/baseq3/textures/gothic_trim/supportborder.data.o \
	bsp/baseq3/textures/gothic_trim/tower_top.data.o \
	bsp/katdm3/textures/katdm3/kt_rock_1f_rot_shiny.data.o \
	bsp/katdm3/textures/katdm3/kattex-3-stone-bmp_rot.data.o \
	bsp/katdm3/textures/katdm3/kattex-3-stone-bmp_fade.data.o \
	bsp/katdm3/textures/katdm3/kt_wood_rough.data.o \
	bsp/katdm3/models/mapobjects/kt_torch/kt_torch.data.o \
	bsp/baseq3/textures/sfx/flame1.data.o \
	bsp/katdm3/models/mapobjects/oak/oakblaetter.data.o \
	bsp/baseq3/models/mapobjects/skel/skel.data.o \
	$(LIBGCC)

example/bsp/katdm3.elf: LDSCRIPT = $(LIB)/main.lds
example/bsp/katdm3.elf: $(START_OBJ) $(BSP_KATDM3_OBJ)
