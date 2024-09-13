all: $(patsubst %.cpp,%.elf,$(wildcard example/*.cpp))

phony:

example/arm/%.bin: phony
	make -C example/arm $*.bin

include common.mk

geometry/%.hpp: geometry/%.obj
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@

%.data.h: %.data
	$(BUILD_BINARY_H)

%.data.pal.h: %.data.pal
	$(BUILD_BINARY_H)

%.alpha.h: %.alpha
	$(BUILD_BINARY_H)

%.alpha.o: %.alpha
	$(BUILD_BINARY_O)

%.alpha.pal.h: %.alpha.pal
	$(BUILD_BINARY_H)

%.alpha.pal.o: %.alpha.pal
	$(BUILD_BINARY_O)

%.vq.h: %.vq
	$(BUILD_BINARY_H)

%.vq.o: %.vq
	$(BUILD_BINARY_O)

build-fonts:
	./tools/ttf_outline 20 7f 20 0 little /usr/share/fonts/dejavu/DejaVuSans.ttf dejavusansmono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/dejavu/DejaVuSans.ttf dejavusansmono_mono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/terminus/ter-u20n.otb ter_u20n.data

include example/example.mk
include chess/chess.mk
include text_editor/text_editor.mk

.PHONY: phony
