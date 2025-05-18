example/arm/%.bin: phony
	make -C example/arm $*.bin

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
	sh7091/serial.o \
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
	texture/macaw/macaw.data.o

example/macaw.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw.elf: $(START_OBJ) $(MACAW_OBJ)

MACAW_TWIDDLE_OBJ = \
	example/macaw_twiddle.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/macaw/macaw.data.o

example/macaw_twiddle.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_twiddle.elf: $(START_OBJ) $(MACAW_TWIDDLE_OBJ)

MACAW_CUBE_OBJ = \
	example/macaw_cube.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/macaw/macaw.data.o

example/macaw_cube.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_cube.elf: $(START_OBJ) $(MACAW_CUBE_OBJ)

MACAW_CUBE_RENDER_TO_TEXTURE_OBJ = \
	example/macaw_cube_render_to_texture.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/macaw/macaw.data.o

example/macaw_cube_render_to_texture.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_cube_render_to_texture.elf: $(START_OBJ) $(MACAW_CUBE_RENDER_TO_TEXTURE_OBJ)

MACAW_MULTIPASS_OBJ = \
	example/macaw_multipass.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/macaw/macaw.data.o \
	sh7091/serial.o

example/macaw_multipass.elf: LDSCRIPT = $(LIB)/main.lds
example/macaw_multipass.elf: $(START_OBJ) $(MACAW_MULTIPASS_OBJ)

POPPIES_MOSAIC_OBJ = \
	example/poppies_mosaic.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/poppies/poppies.data.o \
	sh7091/serial.o

example/poppies_mosaic.elf: LDSCRIPT = $(LIB)/main.lds
example/poppies_mosaic.elf: $(START_OBJ) $(POPPIES_MOSAIC_OBJ)

POPPIES_MOSAIC2_OBJ = \
	example/poppies_mosaic2.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/poppies/poppies.data.o \
	sh7091/serial.o

example/poppies_mosaic2.elf: LDSCRIPT = $(LIB)/main.lds
example/poppies_mosaic2.elf: $(START_OBJ) $(POPPIES_MOSAIC2_OBJ)

FONT_BITMAP_OBJ = \
	example/font_bitmap.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	font/sperrypc/sperrypc_8x8.data.o

example/font_bitmap.elf: LDSCRIPT = $(LIB)/main.lds
example/font_bitmap.elf: $(START_OBJ) $(FONT_BITMAP_OBJ)

FONT_OUTLINE_OBJ = \
	example/font_outline.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	font/dejavusansmono/dejavusansmono.data.o

example/font_outline.elf: LDSCRIPT = $(LIB)/main.lds
example/font_outline.elf: $(START_OBJ) $(FONT_OUTLINE_OBJ)

FONT_OUTLINE_PUNCH_THROUGH_OBJ = \
	example/font_outline_punch_through.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	font/dejavusansmono/dejavusansmono_mono.data.o

example/font_outline_punch_through.elf: LDSCRIPT = $(LIB)/main.lds
example/font_outline_punch_through.elf: $(START_OBJ) $(FONT_OUTLINE_PUNCH_THROUGH_OBJ)

TRANSLUCENCY_OBJ = \
	example/translucency.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/macaw/macaw.data.o

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
	font/verite_8x16/verite_8x16.data.o

example/suzanne_profile.elf: LDSCRIPT = $(LIB)/main.lds
example/suzanne_profile.elf: $(START_OBJ) $(SUZANNE_PROFILE_OBJ)

WIFFLE_ATTENUATION_OBJ = \
	example/wiffle_attenuation.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o

example/wiffle_attenuation.elf: LDSCRIPT = $(LIB)/main.lds
example/wiffle_attenuation.elf: $(START_OBJ) $(WIFFLE_ATTENUATION_OBJ)

WIFFLE_SCREEN_SPACE_OBJ = \
	example/wiffle_screen_space.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o \
	sobel_fipr.o \
	sobel.o

example/wiffle_screen_space.elf: LDSCRIPT = $(LIB)/main.lds
example/wiffle_screen_space.elf: $(START_OBJ) $(WIFFLE_SCREEN_SPACE_OBJ)

WIFFLE_SCREEN_SPACE_STORE_QUEUE_OBJ = \
	example/wiffle_screen_space_store_queue.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o \
	sobel_fipr_store_queue.o

example/wiffle_screen_space_store_queue.elf: LDSCRIPT = $(LIB)/main.lds
example/wiffle_screen_space_store_queue.elf: $(START_OBJ) $(WIFFLE_SCREEN_SPACE_STORE_QUEUE_OBJ)

WIFFLE_SCREEN_SPACE_STORE_QUEUE2_OBJ = \
	example/wiffle_screen_space_store_queue2.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	sh7091/serial.o \
	sobel_fipr_store_queue2.o \
	$(LIBGCC)

example/wiffle_screen_space_store_queue2.elf: LDSCRIPT = $(LIB)/main.lds
example/wiffle_screen_space_store_queue2.elf: $(START_OBJ) $(WIFFLE_SCREEN_SPACE_STORE_QUEUE2_OBJ)

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
	texture/wolf/wolf.data.o \
	texture/macaw/macaw.data.o \
	maple/maple.o \
	sh7091/serial.o \
	$(LIBGCC)

example/modifier_volume_with_two_volumes.elf: LDSCRIPT = $(LIB)/main.lds
example/modifier_volume_with_two_volumes.elf: $(START_OBJ) $(MODIFIER_VOLUME_WITH_TWO_VOLUMES_OBJ)

BLEND_OBJ = \
	example/blend.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	texture/wolf/wolf.data.o \
	texture/macaw/macaw.data.o \
	maple/maple.o \
	sh7091/serial.o \
	$(LIBGCC)

example/blend.elf: LDSCRIPT = $(LIB)/main.lds
example/blend.elf: $(START_OBJ) $(BLEND_OBJ)

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
	texture/macaw/macaw.data.o \
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
	texture/wink/wink.data.o

example/maple_wink.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_wink.elf: $(START_OBJ) $(MAPLE_WINK_OBJ)

MAPLE_FONT_OBJ = \
	example/maple_font.o \
	holly/video_output.o \
	rgb.o \
	sh7091/serial.o \
	maple/maple.o \
	font/portfolio_6x8/portfolio_6x8.data.o

example/maple_font.elf: LDSCRIPT = $(LIB)/main.lds
example/maple_font.elf: $(START_OBJ) $(MAPLE_FONT_OBJ)

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
	serial_load.o \
	crc32.o
#	maple/maple.o \
#	font/portfolio_6x8/portfolio_6x8.data.o \

example/serial_transfer.elf: LDSCRIPT = $(LIB)/loader.lds
example/serial_transfer.elf: $(START_OBJ) $(SERIAL_TRANSFER_OBJ)

SERIAL_DMA_OBJ = \
	example/serial_dma.o \
	sh7091/serial.o

example/serial_dma.elf: LDSCRIPT = $(LIB)/main.lds
example/serial_dma.elf: $(START_OBJ) $(SERIAL_DMA_OBJ)

INTERRUPT_OBJ = \
	example/interrupt.o \
	example/illslot.o \
	sh7091/serial.o

example/interrupt.elf: LDSCRIPT = $(LIB)/main.lds
example/interrupt.elf: $(START_OBJ) $(INTERRUPT_OBJ)

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

GDROM_JVM_BOOT_OBJ = \
	example/gdrom_jvm_boot.o \
	sh7091/serial.o

example/gdrom_jvm_boot.elf: LDSCRIPT = $(LIB)/alt.lds
example/gdrom_jvm_boot.elf: $(START_OBJ) $(GDROM_JVM_BOOT_OBJ)

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
	texture/bbb/bbb1.data.o \
	texture/bbb/bbb2.data.o \
	texture/bbb/bbb4.data.o \
	texture/bbb/bbb8.data.o \
	texture/bbb/bbb16.data.o \
	texture/bbb/bbb32.data.o \
	texture/bbb/bbb64.data.o \
	texture/bbb/bbb128.data.o \
	texture/bbb/bbb256.data.o \
	texture/bbb/bbb512.data.o \
	texture/bbb/bbb1024.data.o \
	sh7091/serial.o

example/texture_filtering.elf: LDSCRIPT = $(LIB)/main.lds
example/texture_filtering.elf: $(START_OBJ) $(TEXTURE_FILTERING_OBJ)

TEXTURE_FILTERING_MAPLE_OBJ = \
	example/texture_filtering_maple.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	texture/bbb/bbb1.data.o \
	texture/bbb/bbb2.data.o \
	texture/bbb/bbb4.data.o \
	texture/bbb/bbb8.data.o \
	texture/bbb/bbb16.data.o \
	texture/bbb/bbb32.data.o \
	texture/bbb/bbb64.data.o \
	texture/bbb/bbb128.data.o \
	texture/bbb/bbb256.data.o \
	texture/bbb/bbb512.data.o \
	texture/bbb/bbb1024.data.o \
	maple/maple.o \
	sh7091/serial.o \
	font/font_bitmap.o \
	font/verite_8x16/verite_8x16.data.o

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
	texture/wolf2/wolf2.data.o \
	texture/wolf2/wolf2.data.pal.o \
	texture/strawberry/strawberry.data.o \
	texture/strawberry/strawberry.data.pal.o \
	$(LIBGCC)

example/sierpinski.elf: LDSCRIPT = $(LIB)/main.lds
example/sierpinski.elf: $(START_OBJ) $(SIERPINSKI_OBJ)

TETRAHEDRON_OBJ = \
	example/tetrahedron.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o

example/tetrahedron.elf: LDSCRIPT = $(LIB)/main.lds
example/tetrahedron.elf: $(START_OBJ) $(TETRAHEDRON_OBJ)

CUBE_TEXTURED_OBJ = \
	example/cube_textured.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	texture/cube/wn.data.o \
	texture/cube/wn.data.pal.o

example/cube_textured.elf: LDSCRIPT = $(LIB)/main.lds
example/cube_textured.elf: $(START_OBJ) $(CUBE_TEXTURED_OBJ)

CUBE_VQ_OBJ = \
	example/cube_vq.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	texture/panda/panda.vq.o

example/cube_vq.elf: LDSCRIPT = $(LIB)/main.lds
example/cube_vq.elf: $(START_OBJ) $(CUBE_VQ_OBJ)

CUBE_VQ_RECTANGULAR_OBJ = \
	example/cube_vq_rectangular.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	texture/panda/panda_rectangular.vq.o

example/cube_vq_rectangular.elf: LDSCRIPT = $(LIB)/main.lds
example/cube_vq_rectangular.elf: $(START_OBJ) $(CUBE_VQ_RECTANGULAR_OBJ)

SHEIK_OBJ = \
	example/sheik.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/sheik/sheik_00.data.o \
	model/sheik/xc_eye01.data.o

example/sheik.elf: LDSCRIPT = $(LIB)/main.lds
example/sheik.elf: $(START_OBJ) $(SHEIK_OBJ)

SHEIK_VQ_OBJ = \
	example/sheik_vq.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/sheik/sheik_00.alpha.o \
	model/sheik/sheik_00.alpha.pal.o \
	model/sheik/sheik_00.vq.o \
	model/sheik/xc_eye01.data.o

example/sheik_vq.elf: LDSCRIPT = $(LIB)/main.lds
example/sheik_vq.elf: $(START_OBJ) $(SHEIK_VQ_OBJ)

VGA_TIMING_OBJ = \
	example/vga_timing.o \
	holly/video_output.o

example/vga_timing.elf: LDSCRIPT = $(LIB)/main.lds
example/vga_timing.elf: $(START_OBJ) $(VGA_TIMING_OBJ)

G2_BUS_OBJ = \
	example/g2_bus.o \
	sh7091/serial.o

example/g2_bus.elf: LDSCRIPT = $(LIB)/main.lds
example/g2_bus.elf: $(START_OBJ) $(G2_BUS_OBJ)

MEMORY_MAP_OBJ = \
	example/memory_map.o \
	sh7091/serial.o

example/memory_map.elf: LDSCRIPT = $(LIB)/main.lds
example/memory_map.elf: $(START_OBJ) $(MEMORY_MAP_OBJ)

TRIANGLE_GOURAUD_OBJ = \
	example/triangle_gouraud.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o

example/triangle_gouraud.elf: LDSCRIPT = $(LIB)/main.lds
example/triangle_gouraud.elf: $(START_OBJ) $(TRIANGLE_GOURAUD_OBJ)

SPRITE_GOURAUD_OBJ = \
	example/sprite_gouraud.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o

example/sprite_gouraud.elf: LDSCRIPT = $(LIB)/main.lds
example/sprite_gouraud.elf: $(START_OBJ) $(SPRITE_GOURAUD_OBJ)

TEXTURE_MEMORY_OBJ = \
	example/texture_memory.o \
	sh7091/serial.o

example/texture_memory.elf: LDSCRIPT = $(LIB)/main.lds
example/texture_memory.elf: $(START_OBJ) $(TEXTURE_MEMORY_OBJ)

HOLLY_RECV_DMA_OBJ = \
	example/holly_recv_dma.o \
	sh7091/serial.o

example/holly_recv_dma.elf: LDSCRIPT = $(LIB)/main.lds
example/holly_recv_dma.elf: $(START_OBJ) $(HOLLY_RECV_DMA_OBJ)

FIPR_OBJ = \
	example/fipr.o \
	fipr.o \
	sobel_fipr.o \
	sh7091/serial.o

example/fipr.elf: LDSCRIPT = $(LIB)/main.lds
example/fipr.elf: $(START_OBJ) $(FIPR_OBJ)

ORA_OBJ = \
	example/ora.o \
	sh7091/serial.o

example/ora.elf: LDSCRIPT = $(LIB)/main.lds
example/ora.elf: $(START_OBJ) $(ORA_OBJ)

TESTSCENE_OBJ = \
	example/testscene.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/testscene/texture/texBrick.data.o \
	model/testscene/texture/texFoliage.data.o \
	model/testscene/texture/texGrass.data.o \
	model/testscene/texture/texGrassClump.data.o \
	model/testscene/texture/texRock.data.o \
	model/testscene/texture/texWater.data.o


example/testscene.elf: LDSCRIPT = $(LIB)/main.lds
example/testscene.elf: $(START_OBJ) $(TESTSCENE_OBJ)

TESTGROUND_OBJ = \
	example/testground.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/testground/maskGround.data.o \
	model/testground/texGrass.data.o \
	model/testground/texGrass2.data.o \
	model/testground/texRock.data.o

example/testground.elf: LDSCRIPT = $(LIB)/main.lds
example/testground.elf: $(START_OBJ) $(TESTGROUND_OBJ)

ELIZABETH_OBJ = \
	example/elizabeth.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/elizabeth/elizabeth_sword_mat_emissive.data.o \
	model/elizabeth/elizabeth_mat_emissive.data.o

example/elizabeth.elf: LDSCRIPT = $(LIB)/main.lds
example/elizabeth.elf: $(START_OBJ) $(ELIZABETH_OBJ)

SPECK_OBJ = \
	example/speck.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/speck/speck.data.o \
	model/speck/white.data.o

example/speck.elf: LDSCRIPT = $(LIB)/main.lds
example/speck.elf: $(START_OBJ) $(SPECK_OBJ)

MODIFIER_VOLUME_CUBE_OBJ = \
	example/modifier_volume_cube.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	font/terminus/ter_u32n.data.o \
	$(LIBGCC)

example/modifier_volume_cube.elf: LDSCRIPT = $(LIB)/main.lds
example/modifier_volume_cube.elf: $(START_OBJ) $(MODIFIER_VOLUME_CUBE_OBJ)

DRAGON_OBJ = \
	example/dragon.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/dragon/dragon.data.o \
	model/dragon/dragon.data.pal.o \
	model/dragon/chrome.data.o \
	$(LIBGCC)

example/dragon.elf: LDSCRIPT = $(LIB)/main.lds
example/dragon.elf: $(START_OBJ) $(DRAGON_OBJ)

CASTLE_OBJ = \
	example/castle.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/castle/castlest.data.o \
	model/castle/gothic3.data.o \
	model/castle/oldbric.data.o \
	model/castle/shingle.data.o \
	model/castle/stone2.data.o \
	$(LIBGCC)

example/castle.elf: LDSCRIPT = $(LIB)/main.lds
example/castle.elf: $(START_OBJ) $(CASTLE_OBJ)

GRADIENT_OBJ = \
	example/gradient.o \
	texture/gradient/gradient.data.o \
	example/moai.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	$(LIBGCC)

example/gradient.elf: LDSCRIPT = $(LIB)/main.lds
example/gradient.elf: $(START_OBJ) $(GRADIENT_OBJ)

MOAI_OBJ = \
	example/moai.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	$(LIBGCC)

example/moai.elf: LDSCRIPT = $(LIB)/main.lds
example/moai.elf: $(START_OBJ) $(MOAI_OBJ)

HAUNTED_MANSION_OBJ = \
	example/haunted_mansion.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	$(LIBGCC)

example/haunted_mansion.elf: LDSCRIPT = $(LIB)/main.lds
example/haunted_mansion.elf: $(START_OBJ) $(HAUNTED_MANSION_OBJ)

BEAR_OBJ = \
	example/bear.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	model/bear/bear.data.o \
	$(LIBGCC)

example/bear.elf: LDSCRIPT = $(LIB)/main.lds
example/bear.elf: $(START_OBJ) $(BEAR_OBJ)

FOG_OBJ = \
	example/fog.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	$(LIBGCC)

example/fog.elf: LDSCRIPT = $(LIB)/main.lds
example/fog.elf: $(START_OBJ) $(FOG_OBJ)

BOIDS_OBJ = \
	example/boids.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	$(LIBGCC)

example/boids.elf: LDSCRIPT = $(LIB)/main.lds
example/boids.elf: $(START_OBJ) $(BOIDS_OBJ)

MOD_OBJ = \
	example/mod.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	mod/mod.o \
	mod/getfunk/getfunk.mod.o \
	$(LIBGCC)

example/mod.elf: LDSCRIPT = $(LIB)/main.lds
example/mod.elf: $(START_OBJ) $(MOD_OBJ)

DOOR_OBJ = \
	example/door.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	printf/unparse.o \
	$(LIBGCC)

example/door.elf: LDSCRIPT = $(LIB)/main.lds
example/door.elf: $(START_OBJ) $(DOOR_OBJ)

BUMP_OBJ = \
	example/bump.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	texture/bump/bump.data.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	font/font_bitmap.o \
	font/verite_8x16/verite_8x16.data.o \
	$(LIBGCC)

example/bump.elf: LDSCRIPT = $(LIB)/main.lds
example/bump.elf: $(START_OBJ) $(BUMP_OBJ)

LIGHTING_OBJ = \
	example/lighting.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	printf/unparse.o \
	$(LIBGCC)

example/lighting.elf: LDSCRIPT = $(LIB)/main.lds
example/lighting.elf: $(START_OBJ) $(LIGHTING_OBJ)

LIGHTING_MAPS_OBJ = \
	example/lighting_maps.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	printf/unparse.o \
	texture/container/container2.data.o \
	texture/container/container2_specular.data.o \
	$(LIBGCC)

example/lighting_maps.elf: LDSCRIPT = $(LIB)/main.lds
example/lighting_maps.elf: $(START_OBJ) $(LIGHTING_MAPS_OBJ)

MD5_OBJ = \
	example/md5.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	texture/container/container2.data.o \
	texture/container/container2_specular.data.o \
	model/boblamp/guard1_body.data.o \
	model/boblamp/guard1_face.data.o \
	model/boblamp/guard1_helmet.data.o \
	model/boblamp/iron_grill.data.o \
	model/boblamp/round_grill.data.o \
	$(LIBGCC)

example/md5.elf: LDSCRIPT = $(LIB)/main.lds
example/md5.elf: $(START_OBJ) $(MD5_OBJ)

BLOOM_OBJ = \
	example/bloom.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	gauss.o \
	model/bloom_scene/wood.data.o \
	model/bloom_scene/container2.data.o \
	$(LIBGCC)

example/bloom.elf: LDSCRIPT = $(LIB)/main.lds
example/bloom.elf: $(START_OBJ) $(BLOOM_OBJ)

BLOOM_LIGHTMAP_OBJ = \
	example/bloom_lightmap.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	holly/video_output.o \
	sh7091/serial.o \
	maple/maple.o \
	sh7091/c_serial.o \
	printf/printf.o \
	printf/unparse.o \
	printf/parse.o \
	gauss.o \
	model/bloom_lightmap/container2.vq.o \
	model/bloom_lightmap/container_lightmap.vq.o \
	model/bloom_lightmap/floor_lightmap.vq.o \
	model/bloom_lightmap/wood.vq.o \
	$(LIBGCC)

example/bloom_lightmap.elf: LDSCRIPT = $(LIB)/main.lds
example/bloom_lightmap.elf: $(START_OBJ) $(BLOOM_LIGHTMAP_OBJ)
