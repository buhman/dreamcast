CFLAGS = -Og -g -gdwarf-4 -Wall -Wextra -Werror -Wfatal-errors -ggdb -Wno-error=unused-parameter -Wno-error=unused-variable -fstack-protector-strong
CXXFLAGS = -std=c++23

CFLAGS += $(shell pkg-config --cflags freetype2)
LDFLAGS = $(shell pkg-config --libs freetype2)

all: ttf-outline

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

%: %.o
	$(CXX) $(LDFLAGS) $^ -o $@

ttf-outline: ttf-outline.o 2d-pack.o

clean:
	rm -f *.o ttf-convert ttf-bitmap

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean
