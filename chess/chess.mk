CHESS_OBJ = \
	chess/main.o \
	chess/render.o \
	chess/chess.o \
	chess/input.o \
	holly/video_output.o \
	holly/core.o \
	holly/region_array.o \
	holly/background.o \
	holly/ta_fifo_polygon_converter.o \
	maple/maple.o \
	sh7091/serial.o \
	$(LIBGCC)

CHESS_GEOMETRY = \
	chess/bishop.hpp \
	chess/king.hpp \
	chess/knight.hpp \
	chess/pawn.hpp \
	chess/queen.hpp \
	chess/rook.hpp \
	chess/square.hpp \
	chess/circle.hpp \
	chess/cursor.hpp

chess/render.o: chess/render.cpp $(CHESS_GEOMETRY)

chess/chess.elf: LDSCRIPT = $(LIB)/main.lds
chess/chess.elf: $(START_OBJ) $(CHESS_OBJ)

chess/%.hpp: chess/%.obj tools/obj_to_cpp.py
	PYTHONPATH=regs/gen python tools/obj_to_cpp.py $< > $@.tmp
	mv $@.tmp $@
