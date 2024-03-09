all: $(patsubst %.cpp,%.elf,$(wildcard example/*.cpp))

phony:

example/arm/%.bin: phony
	make -C example/arm $*.bin

include common.mk

geometry/%.hpp: geometry/%.obj
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@

include example/example.mk

.PHONY: phony
