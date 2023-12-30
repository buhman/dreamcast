SPRITE_OBJ = \
	example/sprite.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/sprite.elf: LDSCRIPT = $(LIB)/alt.lds
example/sprite.elf: $(START_OBJ) $(SPRITE_OBJ)

MACAW_OBJ = \
	example/macaw.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw.elf: $(START_OBJ) $(MACAW_OBJ)

MACAW_TWIDDLE_OBJ = \
	example/macaw_twiddle.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_twiddle.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_twiddle.elf: $(START_OBJ) $(MACAW_TWIDDLE_OBJ)

FONT_BITMAP_OBJ = \
	example/font_bitmap.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	sperrypc.data.o

example/font_bitmap.elf: LDSCRIPT = $(LIB)/alt.lds
example/font_bitmap.elf: $(START_OBJ) $(FONT_BITMAP_OBJ)

FONT_OUTLINE_OBJ = \
	example/font_outline.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	dejavusansmono.data.o

example/font_outline.elf: LDSCRIPT = $(LIB)/alt.lds
example/font_outline.elf: $(START_OBJ) $(FONT_OUTLINE_OBJ)

FONT_OUTLINE_PUNCH_THROUGH_OBJ = \
	example/font_outline_punch_through.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	dejavusansmono_mono.data.o

example/font_outline_punch_through.elf: LDSCRIPT = $(LIB)/alt.lds
example/font_outline_punch_through.elf: $(START_OBJ) $(FONT_OUTLINE_PUNCH_THROUGH_OBJ)

MACAW_MULTIPASS_OBJ = \
	example/macaw_multipass.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_multipass.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_multipass.elf: $(START_OBJ) $(MACAW_MULTIPASS_OBJ)

TRANSLUCENCY_OBJ = \
	example/translucency.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/translucency.elf: LDSCRIPT = $(LIB)/alt.lds
example/translucency.elf: $(START_OBJ) $(TRANSLUCENCY_OBJ)

CUBE_OBJ = \
	example/cube.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/cube.elf: LDSCRIPT = $(LIB)/alt.lds
example/cube.elf: $(START_OBJ) $(CUBE_OBJ)

ICOSPHERE_OBJ = \
	example/icosphere.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/icosphere.elf: LDSCRIPT = $(LIB)/alt.lds
example/icosphere.elf: $(START_OBJ) $(ICOSPHERE_OBJ)

WIFFLE_ATTENUATION_OBJ = \
	example/wiffle_attenuation.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/wiffle_attenuation.elf: LDSCRIPT = $(LIB)/alt.lds
example/wiffle_attenuation.elf: $(START_OBJ) $(WIFFLE_ATTENUATION_OBJ)

MODIFIER_VOLUME_OBJ = \
	example/modifier_volume.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/modifier_volume.elf: LDSCRIPT = $(LIB)/alt.lds
example/modifier_volume.elf: $(START_OBJ) $(MODIFIER_VOLUME_OBJ)

MACAW_CUBE_OBJ = \
	example/macaw_cube.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_cube.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_cube.elf: $(START_OBJ) $(MACAW_CUBE_OBJ)

MACAW_CUBE_RENDER_TO_TEXTURE_OBJ = \
	example/macaw_cube_render_to_texture.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	macaw.data.o

example/macaw_cube_render_to_texture.elf: LDSCRIPT = $(LIB)/alt.lds
example/macaw_cube_render_to_texture.elf: $(START_OBJ) $(MACAW_CUBE_RENDER_TO_TEXTURE_OBJ)

MAPLE_DEVICE_REQUEST_OBJ = \
	example/maple_device_request.o \
	vga.o \
	serial.o \
	maple/maple.o

example/maple_device_request.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_device_request.elf: $(START_OBJ) $(MAPLE_DEVICE_REQUEST_OBJ)

MAPLE_CONTROLLER_OBJ = \
	example/maple_controller.o \
	vga.o \
	serial.o \
	maple/maple.o

example/maple_controller.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_controller.elf: $(START_OBJ) $(MAPLE_CONTROLLER_OBJ)

MAPLE_WINK_OBJ = \
	example/maple_wink.o \
	vga.o \
	rgb.o \
	serial.o \
	maple/maple.o \
	wink.data.o

example/maple_wink.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_wink.elf: $(START_OBJ) $(MAPLE_WINK_OBJ)

MAPLE_VIBRATOR_OBJ = \
	example/maple_vibrator.o \
	vga.o \
	rgb.o \
	serial.o \
	maple/maple.o

example/maple_vibrator.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_vibrator.elf: $(START_OBJ) $(MAPLE_VIBRATOR_OBJ)

MAPLE_ANALOG_OBJ = \
	example/maple_analog.o \
	vga.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	serial.o \
	maple/maple.o

example/maple_analog.elf: LDSCRIPT = $(LIB)/alt.lds
example/maple_analog.elf: $(START_OBJ) $(MAPLE_ANALOG_OBJ)

SERIAL_TRANSFER_OBJ = \
	example/serial_transfer.o \
	serial_load.o

example/serial_transfer.elf: LDSCRIPT = $(LIB)/alt.lds
example/serial_transfer.elf: $(START_OBJ) $(SERIAL_TRANSFER_OBJ)
