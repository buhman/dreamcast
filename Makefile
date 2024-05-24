all: $(patsubst %.cpp,%.elf,$(wildcard example/*.cpp))

phony:

example/arm/%.bin: phony
	make -C example/arm $*.bin

include common.mk

geometry/%.hpp: geometry/%.obj
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@

build-fonts:
	./tools/ttf_outline 20 7f 20 0 little /usr/share/fonts/dejavu/DejaVuSans.ttf dejavusansmono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/dejavu/DejaVuSans.ttf dejavusansmono_mono.data
	./tools/ttf_outline 20 7f 20 1 little /usr/share/fonts/terminus/ter-u20n.otb ter_u20n.data

include example/example.mk
include text_editor/text_editor.mk
include snake/snake.mk
include pokemon/pokemon.mk

.PHONY: phony
