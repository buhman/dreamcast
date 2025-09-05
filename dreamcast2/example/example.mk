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

TRIANGLE_CORE_FULLSCREEN_OBJ = \
	example/triangle_core_fullscreen.o

example/triangle_core_fullscreen.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_core_fullscreen.elf: $(START_OBJ) $(TRIANGLE_CORE_FULLSCREEN_OBJ)

TEXTURED_BACKGROUND_FULLSCREEN_OBJ = \
	example/textured_background_fullscreen.o

example/textured_background_fullscreen.elf: LDSCRIPT = $(LIB)/main.lds
example/textured_background_fullscreen.elf: $(START_OBJ) $(TEXTURED_BACKGROUND_FULLSCREEN_OBJ)

TRIANGLE_TA_OBJ = \
	example/triangle_ta.o

example/triangle_ta.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_ta.elf: $(START_OBJ) $(TRIANGLE_TA_OBJ)

TRIANGLE_TA_FULLSCREEN_OBJ = \
	holly/core/region_array.o \
	example/triangle_ta_fullscreen.o

example/triangle_ta_fullscreen.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_ta_fullscreen.elf: $(START_OBJ) $(TRIANGLE_TA_FULLSCREEN_OBJ)

TRIANGLE_TA_LIST_CONT_OBJ = \
	example/triangle_ta_list_cont.o

example/triangle_ta_list_cont.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_ta_list_cont.elf: $(START_OBJ) $(TRIANGLE_TA_LIST_CONT_OBJ)

CUBE_TA_FULLSCREEN_TEXTURED_OBJ = \
	holly/core/region_array.o \
	example/cube_ta_fullscreen_textured.o

example/cube_ta_fullscreen_textured.elf: LDSCRIPT = $(LIB)/main.lds
example/cube_ta_fullscreen_textured.elf: $(START_OBJ) $(CUBE_TA_FULLSCREEN_TEXTURED_OBJ)

SUZANNE_TRIANGLE_STRIPS_OBJ = \
	holly/core/region_array.o \
	example/suzanne_triangle_strips.o

example/suzanne_triangle_strips.elf: LDSCRIPT = $(LIB)/main.lds
example/suzanne_triangle_strips.elf: $(START_OBJ) $(SUZANNE_TRIANGLE_STRIPS_OBJ)

DEFERRED_SHADING_OBJ = \
	holly/core/region_array.o \
	example/deferred_shading.o

example/deferred_shading.elf: LDSCRIPT = $(LIB)/main.lds
example/deferred_shading.elf: $(START_OBJ) $(DEFERRED_SHADING_OBJ)

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

SIERPINSKI_TETRAHEDRON_FSAA_OBJ = \
	holly/core/region_array.o \
	example/sierpinski_tetrahedron_fsaa.o

example/sierpinski_tetrahedron_fsaa.elf: LDSCRIPT = $(LIB)/main.lds
example/sierpinski_tetrahedron_fsaa.elf: $(START_OBJ) $(SIERPINSKI_TETRAHEDRON_FSAA_OBJ)

SIERPINSKI_TETRAHEDRON_FSAA_YSCALER_OBJ = \
	holly/core/region_array.o \
	example/sierpinski_tetrahedron_fsaa_yscaler.o

example/sierpinski_tetrahedron_fsaa_yscaler.elf: LDSCRIPT = $(LIB)/main.lds
example/sierpinski_tetrahedron_fsaa_yscaler.elf: $(START_OBJ) $(SIERPINSKI_TETRAHEDRON_FSAA_YSCALER_OBJ)

FONT_OBJ = \
	holly/core/region_array.o \
	example/font.o

example/font.elf: LDSCRIPT = $(LIB)/main.lds
example/font.elf: $(START_OBJ) $(FONT_OBJ)

FONT_SERIAL_OBJ = \
	holly/core/region_array.o \
	example/font_serial.o

example/font_serial.elf: LDSCRIPT = $(LIB)/main.lds
example/font_serial.elf: $(START_OBJ) $(FONT_SERIAL_OBJ)

MAPLE_DEVICE_REQUEST_OBJ = \
	example/maple_device_request.o

example/maple_device_request.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_device_request.elf: $(START_OBJ) $(MAPLE_DEVICE_REQUEST_OBJ)

MAPLE_GET_CONDITION_OBJ = \
	example/maple_get_condition.o

example/maple_get_condition.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_get_condition.elf: $(START_OBJ) $(MAPLE_GET_CONDITION_OBJ)

MAPLE_KEYBOARD_SERIAL_FORWARD_OBJ = \
	example/maple_keyboard_serial_forward.o

example/maple_keyboard_serial_forward.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_keyboard_serial_forward.elf: $(START_OBJ) $(MAPLE_KEYBOARD_SERIAL_FORWARD_OBJ)
