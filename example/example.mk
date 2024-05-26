VIDEO_OUTPUT_OBJ = \
	example/video_output.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o

example/video_output.elf: LDSCRIPT = $(LIB)/main.lds
example/video_output.elf: $(START_OBJ) $(VIDEO_OUTPUT_OBJ)

SPRITE_OBJ = \
	example/sprite.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	$(LIBGCC)

example/sprite.elf: LDSCRIPT = $(LIB)/main.lds
example/sprite.elf: $(START_OBJ) $(SPRITE_OBJ)

MACAW_OBJ = \
	example/macaw.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	macaw.data.o

example/macaw.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw.elf: $(START_OBJ) $(MACAW_OBJ)

MACAW_TWIDDLE_OBJ = \
	example/macaw_twiddle.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_twiddle.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_twiddle.elf: $(START_OBJ) $(MACAW_TWIDDLE_OBJ)

FONT_BITMAP_OBJ = \
	example/font_bitmap.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sperrypc_8x8.data.o

example/font_bitmap.elf: LDSCRIPT = $(LIB)/main.lds
example/font_bitmap.elf: $(START_OBJ) $(FONT_BITMAP_OBJ)

FONT_OUTLINE_OBJ = \
	example/font_outline.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	dejavusansmono.data.o

example/font_outline.elf: LDSCRIPT = $(LIB)/main.lds
example/font_outline.elf: $(START_OBJ) $(FONT_OUTLINE_OBJ)

FONT_OUTLINE_PUNCH_THROUGH_OBJ = \
	example/font_outline_punch_through.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	dejavusansmono_mono.data.o

example/font_outline_punch_through.elf: LDSCRIPT = $(LIB)/main.lds
example/font_outline_punch_through.elf: $(START_OBJ) $(FONT_OUTLINE_PUNCH_THROUGH_OBJ)

MACAW_MULTIPASS_OBJ = \
	example/macaw_multipass.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_multipass.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_multipass.elf: $(START_OBJ) $(MACAW_MULTIPASS_OBJ)

TRANSLUCENCY_OBJ = \
	example/translucency.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/translucency.elf: LDSCRIPT = $(LIB)/main.lds
example/translucency.elf: $(START_OBJ) $(TRANSLUCENCY_OBJ)

CUBE_OBJ = \
	example/cube.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/cube.elf: LDSCRIPT = $(LIB)/main.lds
example/cube.elf: $(START_OBJ) $(CUBE_OBJ)

ICOSPHERE_OBJ = \
	example/icosphere.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/icosphere.elf: LDSCRIPT = $(LIB)/main.lds
example/icosphere.elf: $(START_OBJ) $(ICOSPHERE_OBJ)

SUZANNE_PROFILE_OBJ = \
	example/suzanne_profile.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	font/font_bitmap.o \
	sh7091/serial.o \
	verite_8x16.data.o

example/suzanne_profile.elf: LDSCRIPT = $(LIB)/main.lds
example/suzanne_profile.elf: $(START_OBJ) $(SUZANNE_PROFILE_OBJ)

WIFFLE_ATTENUATION_OBJ = \
	example/wiffle_attenuation.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/wiffle_attenuation.elf: LDSCRIPT = $(LIB)/main.lds
example/wiffle_attenuation.elf: $(START_OBJ) $(WIFFLE_ATTENUATION_OBJ)

MODIFIER_VOLUME_OBJ = \
	example/modifier_volume.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/modifier_volume.elf: LDSCRIPT = $(LIB)/main.lds
example/modifier_volume.elf: $(START_OBJ) $(MODIFIER_VOLUME_OBJ)

MODIFIER_VOLUME_WITH_TWO_VOLUMES_OBJ = \
	example/modifier_volume_with_two_volumes.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	wolf.data.o \
	macaw.data.o \
	maple/maple.o \
	$(LIBGCC)

example/modifier_volume_with_two_volumes.elf: LDSCRIPT = $(LIB)/main.lds
example/modifier_volume_with_two_volumes.elf: $(START_OBJ) $(MODIFIER_VOLUME_WITH_TWO_VOLUMES_OBJ)

HEART_OBJ = \
	example/heart.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o

example/heart.elf: LDSCRIPT = $(LIB)/main.lds
example/heart.elf: $(START_OBJ) $(HEART_OBJ)

VIEWING_SYSTEM_OBJ = \
	example/viewing_system.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	$(LIBGCC)

example/viewing_system.elf: LDSCRIPT = $(LIB)/main.lds
example/viewing_system.elf: $(START_OBJ) $(VIEWING_SYSTEM_OBJ)

MACAW_CUBE_OBJ = \
	example/macaw_cube.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_cube.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_cube.elf: $(START_OBJ) $(MACAW_CUBE_OBJ)

MACAW_CUBE_RENDER_TO_TEXTURE_OBJ = \
	example/macaw_cube_render_to_texture.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	macaw.data.o

example/macaw_cube_render_to_texture.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_cube_render_to_texture.elf: $(START_OBJ) $(MACAW_CUBE_RENDER_TO_TEXTURE_OBJ)

CLIPPING_OBJ = \
	example/clipping.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o

example/clipping.elf: LDSCRIPT = $(LIB)/main.lds
example/clipping.elf: $(START_OBJ) $(CLIPPING_OBJ)

CLIPPING2_OBJ = \
	example/clipping2.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o \
	sh7091/serial.o \
	$(LIBGCC)

example/clipping2.elf: LDSCRIPT = $(LIB)/main.lds
example/clipping2.elf: $(START_OBJ) $(CLIPPING2_OBJ)

CLIPPING_TEXTURED_OBJ = \
	example/clipping_textured.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o \
	sh7091/serial.o \
	macaw.data.o \
	$(LIBGCC)

example/clipping_textured.elf: LDSCRIPT = $(LIB)/main.lds
example/clipping_textured.elf: $(START_OBJ) $(CLIPPING_TEXTURED_OBJ)

MAPLE_DEVICE_REQUEST_OBJ = \
	example/maple_device_request.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o

example/maple_device_request.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_device_request.elf: $(START_OBJ) $(MAPLE_DEVICE_REQUEST_OBJ)

MAPLE_CONTROLLER_OBJ = \
	example/maple_controller.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o

example/maple_controller.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_controller.elf: $(START_OBJ) $(MAPLE_CONTROLLER_OBJ)

MAPLE_STORAGE_OBJ = \
	example/maple_storage.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	$(LIBGCC)

example/maple_storage.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_storage.elf: $(START_OBJ) $(MAPLE_STORAGE_OBJ)

MAPLE_WINK_OBJ = \
	example/maple_wink.o \
	holly/video_output.o \
	rgb.o \
	sh7091/serial.o \
	maple/maple.o \
	wink.data.o

example/maple_wink.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_wink.elf: $(START_OBJ) $(MAPLE_WINK_OBJ)

MAPLE_VIBRATOR_OBJ = \
	example/maple_vibrator.o \
	holly/video_output.o \
	rgb.o \
	sh7091/serial.o \
	maple/maple.o

example/maple_vibrator.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_vibrator.elf: $(START_OBJ) $(MAPLE_VIBRATOR_OBJ)

MAPLE_ANALOG_OBJ = \
	example/maple_analog.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o

example/maple_analog.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_analog.elf: $(START_OBJ) $(MAPLE_ANALOG_OBJ)

MAPLE_MOUSE_OBJ = \
	example/maple_mouse.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o \
	maple/maple.o

example/maple_mouse.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_mouse.elf: $(START_OBJ) $(MAPLE_MOUSE_OBJ)

SERIAL_TRANSFER_OBJ = \
	example/serial_transfer.o \
	sh7091/serial.o \
	serial_load.o

example/serial_transfer.elf: LDSCRIPT = $(LIB)/loader.lds
example/serial_transfer.elf: $(START_OBJ) $(SERIAL_TRANSFER_OBJ)

INTERRUPT_OBJ = \
	example/interrupt.o \
	example/illslot.o \
	sh7091/serial.o

example/interrupt.elf: LDSCRIPT = $(LIB)/main.lds
example/interrupt.elf: $(START_OBJ) $(INTERRUPT_OBJ)

DUMP_OBJECT_LIST_OBJ = \
	example/dump_object_list.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o

example/dump_object_list.elf: LDSCRIPT = $(LIB)/main.lds
example/dump_object_list.elf: $(START_OBJ) $(DUMP_OBJECT_LIST_OBJ)

DUMP_RAM_OBJ = \
	example/dump_ram.o \
	sh7091/serial.o

example/dump_ram.elf: LDSCRIPT = $(LIB)/main.lds
example/dump_ram.elf: $(START_OBJ) $(DUMP_RAM_OBJ)

SOFTWARE_TA_OBJ = \
	example/software_ta.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o \
	libm.o

example/software_ta.elf: LDSCRIPT = $(LIB)/main.lds
example/software_ta.elf: $(START_OBJ) $(SOFTWARE_TA_OBJ)

GDROM_TEST_OBJ = \
	example/gdrom_test.o \
	sh7091/serial.o

example/gdrom_test.elf: LDSCRIPT = $(LIB)/main.lds
example/gdrom_test.elf: $(START_OBJ) $(GDROM_TEST_OBJ)

GDROM_ISO9660_OBJ = \
	example/gdrom_iso9660.o \
	sh7091/serial.o

example/gdrom_iso9660.elf: LDSCRIPT = $(LIB)/main.lds
example/gdrom_iso9660.elf: $(START_OBJ) $(GDROM_ISO9660_OBJ)

AICA_OBJ = \
	example/aica.o \
	sh7091/serial.o \
	example/arm/channel.bin.o

example/aica.elf: LDSCRIPT = $(LIB)/main.lds
example/aica.elf: $(START_OBJ) $(AICA_OBJ)

AICA_GDROM_OBJ = \
	example/aica_gdrom.o \
	sh7091/serial.o \
	example/arm/sh4_interrupt.bin.o

example/aica_gdrom.elf: LDSCRIPT = $(LIB)/main.lds
example/aica_gdrom.elf: $(START_OBJ) $(AICA_GDROM_OBJ)

MAC_SATURATION_OBJ = \
	example/mac_saturation.o \
	example/macl_saturation.o \
	example/macw_saturation.o \
	sh7091/serial.o

example/mac_saturation.elf: LDSCRIPT = $(LIB)/main.lds
example/mac_saturation.elf: $(START_OBJ) $(MAC_SATURATION_OBJ)

TA_INTERROGATION_OBJ = \
	example/ta_interrogation.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o

example/ta_interrogation.elf: LDSCRIPT = $(LIB)/main.lds
example/ta_interrogation.elf: $(START_OBJ) $(TA_INTERROGATION_OBJ)

DECODE_TEST_OBJ = \
	example/decode_test.o \
	sh7091/serial.o

example/decode_test.elf: LDSCRIPT = $(LIB)/main.lds
example/decode_test.elf: $(START_OBJ) $(DECODE_TEST_OBJ)

TEXTURE_FILTERING_OBJ = \
	example/texture_filtering.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	bbb1.data.o \
	bbb2.data.o \
	bbb4.data.o \
	bbb8.data.o \
	bbb16.data.o \
	bbb32.data.o \
	bbb64.data.o \
	bbb128.data.o \
	bbb256.data.o \
	bbb512.data.o \
	bbb1024.data.o

example/texture_filtering.elf: LDSCRIPT = $(LIB)/main.lds
example/texture_filtering.elf: $(START_OBJ) $(TEXTURE_FILTERING_OBJ)

TEXTURE_FILTERING_MAPLE_OBJ = \
	example/texture_filtering_maple.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	bbb1.data.o \
	bbb2.data.o \
	bbb4.data.o \
	bbb8.data.o \
	bbb16.data.o \
	bbb32.data.o \
	bbb64.data.o \
	bbb128.data.o \
	bbb256.data.o \
	bbb512.data.o \
	bbb1024.data.o \
	maple/maple.o \
	sh7091/serial.o \
	font/font_bitmap.o \
	verite_8x16.data.o

example/texture_filtering_maple.elf: LDSCRIPT = $(LIB)/main.lds
example/texture_filtering_maple.elf: $(START_OBJ) $(TEXTURE_FILTERING_MAPLE_OBJ)

LFSR_OBJ = \
	example/lfsr.o \
	holly/core.o \
	holly/video_output.o \
	sh7091/serial.o

example/lfsr.elf: LDSCRIPT = $(LIB)/main.lds
example/lfsr.elf: $(START_OBJ) $(LFSR_OBJ)

TA_TRANSFER_PROFILE_OBJ = \
	example/ta_transfer_profile.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o

example/ta_transfer_profile.elf: LDSCRIPT = $(LIB)/main.lds
example/ta_transfer_profile.elf: $(START_OBJ) $(TA_TRANSFER_PROFILE_OBJ)

VQ_OBJ = \
	example/vq.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o

example/vq.elf: LDSCRIPT = $(LIB)/main.lds
example/vq.elf: $(START_OBJ) $(VQ_OBJ)

SIERPINSKI_OBJ = \
	example/sierpinski.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	wolf2.data.o \
	wolf2.data.pal.o \
	strawberry.data.o \
	strawberry.data.pal.o

example/sierpinski.elf: LDSCRIPT = $(LIB)/main.lds
example/sierpinski.elf: $(START_OBJ) $(SIERPINSKI_OBJ)
