OPT ?= -Og
GENERATED ?=

AARCH = --isa=sh4 --little

CARCH ?= -m4-single-only -ml
CFLAGS += -mfsca -funsafe-math-optimizations -ffast-math

OBJARCH = -O elf32-shl -B sh4

TARGET = sh4-none-elf-

START_OBJ = \
	$(LIB)/start.o \
	$(LIB)/runtime.o \
	$(LIB)/sh7091/cache.o

geometry/%.hpp: geometry/%.obj
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@

build-fonts:
	./tools/ttf_outline 20 7f 20 0 little /usr/share/fonts/dejavu/DejaVuSans.ttf font/dejavusansmono/dejavusansmono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/dejavu/DejaVuSans.ttf font/dejavusansmono/dejavusansmono_mono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/terminus/ter-u20n.otb font/ter_u20n/ter_u20n.data

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

# %.iso: %.bin ip.bin
# 	mkisofs \
# 		-C 0,11702 \
# 		-sysid     "SEGA SEGAKATANA" \
# 		-volid     "SAMPLE_GAME_TITLE" \
# 		-volset    "SAMPLE_GAME_TITLE" \
# 		-publisher "SEGA ENTERPRISES, LTD." \
# 		-preparer  "CRI CD CRAFT VER.2.27" \
# 		-copyright "COPYRIGH.TXT" \
# 		-abstract  "ABSTRACT.TXT" \
# 		-biblio    "BIBLIOGR.TXT" \
# 		-G ip.bin \
# 		-o $@ \
# 		-graft-points \
# 		/1ST_READ.BIN=./$< \
# 		/=./COPYRIGH.TXT \
# 		/=./ABSTRACT.TXT \
# 		/=./BIBLIOGR.TXT

zero.bin:
	dd if=/dev/zero of=$@ bs=2048 count=1

%.iso: %.bin gdrom_jvm_boot.bin zero.bin
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
		-G gdrom_jvm_boot.bin \
		-o $@ \
		-graft-points \
		/0JVM.BIN=./$< \
		/1ST_READ.BIN=zero.bin \
		/=$(LIB)/COPYRIGH.TXT \
		/=$(LIB)/ABSTRACT.TXT \
		/=$(LIB)/BIBLIOGR.TXT

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(iso|o|d|bin|elf|cue|gch|scramble)$$' \
		-exec rm {} \;

phony:

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean phony

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
