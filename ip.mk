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
	$(START_OBJ)

SERIAL_LOAD_OBJ = \
	example/serial_transfer.o \
	sh7091/serial.o \
	serial_load.o \
	maple/maple.o \
	font/portfolio_6x8/portfolio_6x8.data.o \
	crc32.o

GDROM_JVM_BOOT_OBJ = \
	example/gdrom_jvm_boot.o \
	sh7091/serial.o

%.o: %.obj
	$(OBJCOPY) -g \
		--rename-section IP=.text.$* \
		$< $@

serial_load_ip.elf: $(IP_OBJ) $(SERIAL_LOAD_OBJ)
	$(LD) --orphan-handling=error --print-memory-usage -T $(LIB)/ip.lds $^ -o $@

gdrom_jvm_boot_ip.elf: $(IP_OBJ) $(GDROM_JVM_BOOT_OBJ)
	$(LD) --orphan-handling=error --print-memory-usage -T $(LIB)/ip.lds $^ -o $@
