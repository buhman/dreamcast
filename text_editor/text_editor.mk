TEXT_EDITOR_OBJ = \
	text_editor/text_editor.o \
	text_editor/gap_buffer.o \
	text_editor/render.o \
	text_editor/keyboard.o \
	text_editor/transform.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o \
	sh7091/serial.o \
	ter_u20n.data.o

text_editor/text_editor.elf: LDSCRIPT = $(LIB)/main.lds
text_editor/text_editor.elf: $(START_OBJ) $(TEXT_EDITOR_OBJ)
