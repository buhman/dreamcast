MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
DIR := $(dir $(MAKEFILE_PATH))

LIB ?= .
OPT ?= -O2
DEBUG ?= -g -gdwarf-4
GENERATED ?=

AARCH = --isa=sh4 --little
AFLAGS = --fatal-warnings

CARCH = -m4-single-only -ml
CFLAGS += -falign-functions=4 -ffunction-sections -fdata-sections -fshort-enums -ffreestanding -nostdlib
CFLAGS += -Wall -Werror -Wfatal-errors
CFLAGS += -Wno-error=narrowing -Wno-error=unused-variable -Wno-error=array-bounds= -Wno-array-bounds
CFLAGS += -mfsca -funsafe-math-optimizations
CFLAGS += -I$(dir $(MAKEFILE_PATH))
DEPFLAGS = -MMD -E
# --print-gc-sections
LDFLAGS = --gc-sections --no-warn-rwx-segment --print-memory-usage --entry=_start --orphan-handling=error
CXXFLAGS = -std=c++23 -fno-exceptions -fno-non-call-exceptions -fno-rtti -fno-threadsafe-statics

TARGET = sh4-none-elf-
CC = $(TARGET)gcc
CXX = $(TARGET)g++
AS = $(TARGET)as
LD = $(TARGET)ld
OBJCOPY = $(TARGET)objcopy
OBJDUMP = $(TARGET)objdump

LIBGCC = $(shell $(CC) -print-file-name=libgcc.a)

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

START_OBJ = \
	start.o \
	runtime.o \
	sh7091/cache.o

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

%.elf:
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) $^ -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	du -b $@

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

1ST_READ.BIN: example/macaw_multipass.bin
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

regs/%.csv: regs/%.ods
	libreoffice --headless -convert-to csv:"Text - txt - csv (StarCalc)":44,34,76,,,,true --outdir regs/ $<

maple/maple_bus_commands.hpp: regs/maple_bus_commands.csv regs/gen/maple_bus_commands.py
	python regs/gen/maple_bus_commands.py $< > $@

maple/maple_bus_bits.hpp: regs/maple_bus_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

holly/core_bits.hpp: regs/core_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

holly/holly.hpp: regs/holly.csv regs/gen/holly.py
	python regs/gen/holly.py $< > $@

holly/ta_global_parameter.hpp: regs/global_parameter_format.csv regs/gen/ta_parameter_format.py
	python regs/gen/ta_parameter_format.py $< ta_global_parameter > $@

holly/ta_vertex_parameter.hpp: regs/vertex_parameter_format.csv regs/gen/ta_parameter_format.py
	python regs/gen/ta_parameter_format.py $< ta_vertex_parameter > $@

sh7091/sh7091.hpp: regs/sh7091.csv regs/gen/sh7091.py
	python regs/gen/sh7091.py $< > $@

sh7091/sh7091_bits.hpp: regs/sh7091_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(iso|o|d|bin|elf|cue|gch)$$' \
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
