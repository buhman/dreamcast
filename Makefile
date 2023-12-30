all:

include common.mk

geometry/%.hpp: geometry/%.obj
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@

include example/example.mk
