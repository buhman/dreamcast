CFLAGS = -Og -g -gdwarf-4 -Wall -Wextra -Wno-error
CFLAGS += -Wno-error=unused-parameter
CFLAGS += -Wno-error=unused-variable
CFLAGS += -Wno-error=unused-but-set-variable
CFLAGS += -Wno-vla-extension
CXXFLAGS = -std=c++23

PREFIX ?= /usr/local
MANPREFIX = $(PREFIX)/share/man

FREETYPE_CFLAGS = $(shell pkg-config --cflags freetype2)
FREETYPE_LDFLAGS = $(shell pkg-config --libs freetype2)

FTDI_CFLAGS = $(shell pkg-config --cflags libftdi1) -I.
FTDI_LDFLAGS = $(shell pkg-config --libs libftdi1)

all: ttf_outline

crc32.o: crc32.c
	$(CC) -std=gnu2x $(CFLAGS) -I. -c $< -o $@

ttf_%.o: ttf_%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(FREETYPE_CFLAGS) -c $< -o $@

ttf_%: ttf_%.o
	$(CXX) $(LDFLAGS) $(FREETYPE_LDFLAGS) $^ -o $@

ftdi_%.o: ftdi_%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(FTDI_CFLAGS) -c $< -o $@

FTDI_OBJ = \
	ftdi_transfer.o \
	ftdi_maple.o \
	crc32.o

ftdi_transfer: $(FTDI_OBJ)
	$(CXX) $^ -o $@ $(FTDI_LDFLAGS)

ttf_outline: ttf_outline.o ttf_2d_pack.o

clean:
	rm -f *.o ttf_outline

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f ftdi_transfer $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ftdi_transfer
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f ftdi_transfer.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/ftdi_transfer.1

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean install

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
