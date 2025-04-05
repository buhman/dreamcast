IP_OBJ = \
	$(LIB)/systemid.o \
	$(LIB)/toc.o \
	$(LIB)/sg/sg_sec.o \
	$(LIB)/sg_arejp.o \
	$(LIB)/sg_areus.o \
	$(LIB)/sg_areec.o \
	$(LIB)/sg_are00.o \
	$(LIB)/sg_are01.o \
	$(LIB)/sg_are02.o \
	$(LIB)/sg_are03.o \
	$(LIB)/sg_are04.o

SERIAL_LOAD_OBJ = \
	example/serial_transfer.o \
	sh7091/serial.o \
	serial_load.o \
	maple/maple.o \
	font/portfolio_6x8/portfolio_6x8.data.o \
	crc32.o

%.o: %.obj
	$(OBJCOPY) -g \
		--rename-section IP=.text.$* \
		$< $@

serial_load_ip.elf: $(IP_OBJ) $(START_OBJ) $(SERIAL_LOAD_OBJ)
	$(LD) $(LDFLAGS) -L $(LIB) -T $(LIB)/ip.lds $^ -o $@

GDROM_JVM_BOOT_OBJ = \
	$(LIB)/example/gdrom_jvm_boot.o \
	$(LIB)/sh7091/serial.o

gdrom_jvm_boot.elf: $(IP_OBJ) $(START_OBJ) $(GDROM_JVM_BOOT_OBJ)
	$(LD) $(LDFLAGS) -L $(LIB) -T $(LIB)/ip.lds $^ -o $@
