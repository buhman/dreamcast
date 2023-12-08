all: main.elf

include common.mk

MAIN_OBJ = \
	start.o \
	main.o \
	load.o \
	cache.o \
	vga.o \
	rgb.o \
	holly/background.o \
	holly/region_array.o \
	holly/ta_fifo_polygon_converter.o \
	holly/core.o \
	maple.o \
	scene.o \
	macaw.data.o \
	$(LIBGCC)

serial.elf: start.o serial_main.o load.o cache.o
	$(LD) $(LDFLAGS) -T $(LIB)/alt.lds $^ -o $@

main.elf: $(MAIN_OBJ)
	$(LD) $(LDFLAGS) -T $(LIB)/main.lds $^ -o $@

test.elf: $(MAIN_OBJ)
	$(LD) $(LDFLAGS) -T $(LIB)/alt.lds $^ -o $@
