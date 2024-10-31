CFLAGS = -Og -g -gdwarf-4 -Wall -Wextra -Werror -Wfatal-errors -ggdb -Wno-error=unused-parameter -Wno-error=unused-variable -fstack-protector-strong
CXXFLAGS = -std=c++23

FREETYPE_CFLAGS = $(shell pkg-config --cflags freetype2)
FREETYPE_LDFLAGS = $(shell pkg-config --libs freetype2)

FTDI_CFLAGS = $(shell pkg-config --cflags libftdi1) -I..
FTDI_LDFLAGS = $(shell pkg-config --libs libftdi1)

all: ttf_outline

crc32.o: ../crc32.c
	$(CC) -std=gnu2x $(CFLAGS) -I.. -c $< -o $@

ttf_%.o: ttf_%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(FREETYPE_CFLAGS) -c $< -o $@

ttf_%: ttf_%.o
	$(CXX) $(LDFLAGS) $(FREETYPE_LDFLAGS) $^ -o $@

ftdi_%.o: ftdi_%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(FTDI_CFLAGS) -c $< -o $@

ftdi_%: ftdi_%.o crc32.o
	$(CXX) $^ -o $@ $(FTDI_LDFLAGS)

ttf_outline: ttf_outline.o 2d_pack.o

clean:
	rm -f *.o ttf_outline

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
