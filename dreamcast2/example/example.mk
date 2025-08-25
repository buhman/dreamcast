START_OBJ = \
	start.o \
	runtime.o

FRAMEBUFFER_SHADED_OBJ = \
	example/framebuffer_shaded.o

example/framebuffer_shaded.elf: LDSCRIPT = $(LIB)/main.lds
example/framebuffer_shaded.elf: $(START_OBJ) $(FRAMEBUFFER_SHADED_OBJ)

TRIANGLE_CORE_OBJ = \
	holly/core/region_array.o \
	example/triangle_core.o

example/triangle_core.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_core.elf: $(START_OBJ) $(TRIANGLE_CORE_OBJ)
