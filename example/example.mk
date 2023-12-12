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

CUBE_OBJ = \
	example/cube.o \
	vga.o \
	holly/core.o \
	holly/ta_fifo_polygon_converter.o

example/cube.elf: LDSCRIPT = $(LIB)/alt.lds
example/cube.elf: $(START_OBJ) $(CUBE_OBJ)

MAPLE_WINK_OBJ = \
	example/maple_wink.o \
	vga.o \
	rgb.o \
	serial.o \
	maple/maple.o \
	wink.data.o

example/maple_wink.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_wink.elf: $(START_OBJ) $(MAPLE_WINK_OBJ)
