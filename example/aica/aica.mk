AICA_OBJ = \
	example/aica/aica.o \
	sh7091/serial.o \
	example/arm/channel.bin.o

example/aica/aica.elf: LDSCRIPT = $(LIB)/main.lds
example/aica/aica.elf: $(START_OBJ) $(AICA_OBJ)

AICA_XM_OBJ = \
	example/aica/aica_xm.o \
	sh7091/serial.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	xm/milkypack01.xm.o \
	xm/test.xm.o \
	xm/xmtest.xm.o \
	xm/catch_this_rebel.xm.o \
	xm/middle_c.xm.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	$(LIBGCC)

example/aica/aica_xm.elf: LDSCRIPT = $(LIB)/main.lds
example/aica/aica_xm.elf: $(START_OBJ) $(AICA_XM_OBJ)

AICA_GDROM_OBJ = \
	example/aica/aica_gdrom.o \
	sh7091/serial.o \
	example/arm/sh4_interrupt.bin.o

example/aica/aica_gdrom.elf: LDSCRIPT = $(LIB)/main.lds
example/aica/aica_gdrom.elf: $(START_OBJ) $(AICA_GDROM_OBJ)

AICA_GDROM_DFT_OBJ = \
	example/aica/aica_gdrom_dft.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	example/arm/sh4_interrupt.bin.o \
	$(LIBGCC)

example/aica/aica_gdrom_dft.elf: LDSCRIPT = $(LIB)/main.lds
example/aica/aica_gdrom_dft.elf: $(START_OBJ) $(AICA_GDROM_DFT_OBJ)
