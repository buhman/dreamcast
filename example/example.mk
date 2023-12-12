CUBE_OBJ = \
	example/cube.o \
	vga.o

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
