all: main.elf

include common.mk
include example/example.mk

MAIN_OBJ = \
	main.o \
	vga.o \
	rgb.o \
	holly/background.o \
	holly/region_array.o \
	holly/ta_fifo_polygon_converter.o \
	holly/core.o \
	maple/maple.o \
	scene.o \
	macaw.data.o \
	wink.data.o

serial.elf: LDSCRIPT = $(LIB)/alt.lds
serial.elf: $(START_OBJ) serial_main.o load.o

main.elf: LDSCRIPT = $(LIB)/main.lds
main.elf: $(START_OBJ) $(MAIN_OBJ)

test.elf: LDSCRIPT = $(LIB)/alt.lds
test.elf: $(START_OBJ) $(MAIN_OBJ)
