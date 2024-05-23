MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
DIR := $(dir $(MAKEFILE_PATH))

LIB ?= .
OPT ?= -O3
GENERATED ?=

AARCH = --isa=sh4 --little

CARCH = -m4-single-only -ml
CFLAGS += -mfsca -funsafe-math-optimizations -ffast-math
CFLAGS += -I$(dir $(MAKEFILE_PATH))

CXXFLAGS += -std=c++23

OBJARCH = -O elf32-shl -B sh4

TARGET = sh4-none-elf-

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

%.o: %.obj
	$(OBJCOPY) -g \
		--rename-section IP=.text.$* \
		$< $@

ip.elf: $(IP_OBJ)
	$(LD) --orphan-handling=error --print-memory-usage -T $(LIB)/ip.lds $^ -o $@

START_OBJ = \
	start.o \
	runtime.o \
	sh7091/cache.o

include base.mk

sine.pcm: common.mk
	sox \
		--rate 44100 \
		--encoding signed-integer \
		--bits 16 \
		--channels 1 \
		--endian big \
		--null \
		$@.raw \
		synth 100s sin 700 vol -10dB
	mv $@.raw $@

%.scramble: %.bin
	./scramble $< $@

%.iso: %.scramble ip.bin
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
		-G ip.bin \
		-o $@ \
		-graft-points \
		/1ST_READ.BIN=./$< \
		/=./COPYRIGH.TXT \
		/=./ABSTRACT.TXT \
		/=./BIBLIOGR.TXT

%.cdi: %.iso
	./cdi4dc $< $@ >/dev/null

include headers.mk

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(iso|o|d|bin|elf|cue|gch|scramble)$$' \
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
