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
