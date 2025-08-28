START_OBJ = \
	start.o \
	runtime.o

FRAMEBUFFER_SHADED_OBJ = \
	example/framebuffer_shaded.o

example/framebuffer_shaded.elf: LDSCRIPT = $(LIB)/main.lds
example/framebuffer_shaded.elf: $(START_OBJ) $(FRAMEBUFFER_SHADED_OBJ)

TRIANGLE_CORE_OBJ = \
	example/triangle_core.o

example/triangle_core.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_core.elf: $(START_OBJ) $(TRIANGLE_CORE_OBJ)

TRIANGLE_TA_OBJ = \
	example/triangle_ta.o

example/triangle_ta.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_ta.elf: $(START_OBJ) $(TRIANGLE_TA_OBJ)

TRIANGLE_TA_FULLSCREEN_OBJ = \
	holly/core/region_array.o \
	example/triangle_ta_fullscreen.o

example/triangle_ta_fullscreen.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_ta_fullscreen.elf: $(START_OBJ) $(TRIANGLE_TA_FULLSCREEN_OBJ)

CUBE_TA_FULLSCREEN_TEXTURED_OBJ = \
	holly/core/region_array.o \
	example/cube_ta_fullscreen_textured.o

example/cube_ta_fullscreen_textured.elf: LDSCRIPT = $(LIB)/main.lds
example/cube_ta_fullscreen_textured.elf: $(START_OBJ) $(CUBE_TA_FULLSCREEN_TEXTURED_OBJ)

TETRAHEDRON_OBJ = \
	holly/core/region_array.o \
	example/tetrahedron.o

example/tetrahedron.elf: LDSCRIPT = $(LIB)/main.lds
example/tetrahedron.elf: $(START_OBJ) $(TETRAHEDRON_OBJ)

SIERPINSKI_TETRAHEDRON_OBJ = \
	holly/core/region_array.o \
	example/sierpinski_tetrahedron.o

example/sierpinski_tetrahedron.elf: LDSCRIPT = $(LIB)/main.lds
example/sierpinski_tetrahedron.elf: $(START_OBJ) $(SIERPINSKI_TETRAHEDRON_OBJ)
