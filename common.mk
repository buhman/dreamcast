LIB ?= .
OPT ?= -Os
DEBUG ?= -g -gdwarf-4
GENERATED ?=

AARCH = --isa=sh4 --little
AFLAGS = --fatal-warnings

CARCH = -m4-single-only -ml
CFLAGS += -falign-functions=4 -ffunction-sections -fdata-sections -fshort-enums -ffreestanding -nostdlib -Wno-error=narrowing
CFLAGS += -Wall -Werror -Wfatal-errors -Wno-error=unused-variable
CFLAGS += -mfsca -funsafe-math-optimizations
DEPFLAGS = -MMD -E
# --print-gc-sections
LDFLAGS = --gc-sections --no-warn-rwx-segment --print-memory-usage --entry=_start --orphan-handling=error
CXXFLAGS = -std=c++20 -fno-exceptions -fno-non-call-exceptions -fno-rtti -fno-threadsafe-statics

TARGET = sh4-none-elf-
CC = $(TARGET)gcc
CXX = $(TARGET)g++
AS = $(TARGET)as
LD = $(TARGET)ld
OBJCOPY = $(TARGET)objcopy
OBJDUMP = $(TARGET)objdump

define BUILD_BINARY_O
	$(OBJCOPY) \
		-I binary -O elf32-shl -B sh4 \
		--rename-section .data=.data.$(basename $@) \
		$< $@
endef

IP_OBJ = \
	systemid.o \
	toc.o \
	sg/sg_sec.o \
	sg/sg_arejp.o \
	sg/sg_areus.o \
	sg/sg_areec.o \
	sg/sg_are00.o \
	sg/sg_are01.o \
	sg/sg_are02.o \
	sg/sg_are03.o \
	sg/sg_are04.o \
	sg/sg_ini.o \
	sg/aip.o

MAIN_OBJ = \
	start.o \
	main.o \
	load.o \
	cache.o \
	vga.o \
	rgb.o \
	holly/background.o \
	holly/region_array.o \
	holly/ta_parameter.o \
	holly/ta_fifo_polygon_converter.o \
	holly/core.o \
	scene.o

all: main.cdi

%.bin.o: %.bin
	$(BUILD_BINARY_O)

%.o: %.obj
	$(OBJCOPY) -g \
		--rename-section IP=.text.$* \
		$< $@

%.o: %.s
	$(AS) $(AARCH) $(AFLAGS) $(DEBUG) $< -o $@

%.c.d: | $(GENERATED)
	$(CC) $(CARCH) $(CFLAGS) $(OPT) $(DEBUG) $(DEPFLAGS) -c $(basename $@) -MF $@ -o /dev/null

%.o: %.c %.c.d
	$(CC) $(CARCH) $(CFLAGS) $(OPT) $(DEBUG) -c $< -o $@

%.cpp.d: | $(GENERATED)
	$(CXX) $(CARCH) $(CFLAGS) $(CXXFLAGS) $(OPT) $(DEBUG) $(DEPFLAGS) -c $(basename $@) -MF $@ -o /dev/null

%.o: %.cpp %.cpp.d
	$(CXX) $(CARCH) $(CFLAGS) $(CXXFLAGS) $(OPT) $(DEBUG) -c $< -o $@

serial.elf: start.o serial_main.o load.o cache.o
	$(LD) $(LDFLAGS) -T $(LIB)/alt.lds $^ -o $@

main.elf: $(MAIN_OBJ)
	$(LD) $(LDFLAGS) -T $(LIB)/main.lds $^ -o $@

test.elf: $(MAIN_OBJ)
	$(LD) $(LDFLAGS) -T $(LIB)/alt.lds $^ -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

ip.elf: $(IP_OBJ)
	$(LD) --orphan-handling=error --print-memory-usage -T $(LIB)/ip.lds $^ -o $@

audio.pcm:
	sox \
		--rate 44100 \
		--encoding signed-integer \
		--bits 16 \
		--channels 2 \
		--endian little \
		--null \
		$@.raw \
		synth 1 sin 440 vol -10dB
	mv $@.raw $@

1ST_READ.BIN: main.bin
	./scramble $< $@

%.iso: 1ST_READ.BIN ip.bin
	mkisofs \
		-C 0,11702 \
		-sysid     "SEGA SEGAKATANA" \
		-volid     "SAMPLE_GAME_TITLE" \
		-volset    "SAMPLE_GAME_TITLE" \
		-publisher "SEGA ENTERPRISES, LTD." \
		-preparer  "CRI CD CRAFT VER.2.27" \
		-copyright "COPYRIGH.TXT" \
		-abstract  "ABSTRACT.TXT" \
		-biblio    "BIBLIOGR.TXT" \
		-sectype data \
		-G ip.bin \
		-o $@ \
		-graft-points \
		/=./1ST_READ.BIN \
		/=./COPYRIGH.TXT \
		/=./ABSTRACT.TXT \
		/=./BIBLIOGR.TXT

%.cdi: %.iso
	./cdi4dc $< $@ >/dev/null

%.data.o: %.data
	$(BUILD_BINARY_O)

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(iso|o|bin|elf|cue|gch)$$' \
		-exec rm {} \;

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
