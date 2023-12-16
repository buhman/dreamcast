SPRITE_OBJ = \
	example/sprite.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/sprite.elf: LDSCRIPT = $(LIB)/alt.lds
example/sprite.elf: $(START_OBJ) $(SPRITE_OBJ)

MACAW_OBJ = \
	example/macaw.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw.elf: $(START_OBJ) $(MACAW_OBJ)

MACAW_TWIDDLE_OBJ = \
	example/macaw_twiddle.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_twiddle.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_twiddle.elf: $(START_OBJ) $(MACAW_TWIDDLE_OBJ)

FONT_OBJ = \
	example/font.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	sperrypc.data.o

example/font.elf: LDSCRIPT = $(LIB)/alt.lds
example/font.elf: $(START_OBJ) $(FONT_OBJ)

MACAW_MULTIPASS_OBJ = \
	example/macaw_multipass.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_multipass.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_multipass.elf: $(START_OBJ) $(MACAW_MULTIPASS_OBJ)

TRANSLUCENCY_OBJ = \
	example/translucency.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/translucency.elf: LDSCRIPT = $(LIB)/alt.lds
example/translucency.elf: $(START_OBJ) $(TRANSLUCENCY_OBJ)

CUBE_OBJ = \
	example/cube.o \
	vga.o \
	holly/core.o \
	holly/ta_fifo_polygon_converter.o

example/cube.elf: LDSCRIPT = $(LIB)/alt.lds
example/cube.elf: $(START_OBJ) $(CUBE_OBJ)

MAPLE_DEVICE_REQUEST_OBJ = \
	example/maple_device_request.o \
	vga.o \
	serial.o \
	maple/maple.o

example/maple_device_request.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_device_request.elf: $(START_OBJ) $(MAPLE_DEVICE_REQUEST_OBJ)

MAPLE_CONTROLLER_OBJ = \
	example/maple_controller.o \
	vga.o \
	serial.o \
	maple/maple.o

example/maple_controller.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_controller.elf: $(START_OBJ) $(MAPLE_CONTROLLER_OBJ)

MAPLE_WINK_OBJ = \
	example/maple_wink.o \
	vga.o \
	rgb.o \
	serial.o \
	maple/maple.o \
	wink.data.o

example/maple_wink.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_wink.elf: $(START_OBJ) $(MAPLE_WINK_OBJ)
