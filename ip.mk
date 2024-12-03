IP_OBJ = \
	systemid.o \
	toc.o \
	sg/sg_sec.o \
	sg_arejp.o \
	sg_areus.o \
	sg_areec.o \
	sg_are00.o \
	sg_are01.o \
	sg_are02.o \
	sg_are03.o \
	sg_are04.o \
	$(START_OBJ) \
	example/serial_transfer.o \
	sh7091/serial.o \
	serial_load.o \
	maple/maple.o \
	font/portfolio_6x8/portfolio_6x8.data.o \
	crc32.o
#sg_ini.o \
#aip.o

%.o: %.obj
	$(OBJCOPY) -g \
		--rename-section IP=.text.$* \
		$< $@

ip.elf: $(IP_OBJ)
	$(LD) --orphan-handling=error --print-memory-usage -T $(LIB)/ip.lds $^ -o $@
