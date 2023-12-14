MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
DIR := $(dir $(MAKEFILE_PATH))

LIB ?= .
OPT ?= -O0
DEBUG ?= -g -gdwarf-4
GENERATED ?=

AARCH = --isa=sh4 --little
AFLAGS = --fatal-warnings

CARCH = -m4-single-only -ml
CFLAGS += -falign-functions=4 -ffunction-sections -fdata-sections -fshort-enums -ffreestanding -nostdlib
CFLAGS += -Wall -Werror -Wfatal-errors
CFLAGS += -Wno-error=narrowing -Wno-error=unused-variable
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
	cache.o

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
