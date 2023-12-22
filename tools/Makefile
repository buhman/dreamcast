CFLAGS = -Og -g -gdwarf-4 -Wall -Wextra -Werror -Wfatal-errors -ggdb -Wno-error=unused-parameter -Wno-error=unused-variable -fstack-protector-strong
CXXFLAGS = -std=c++23

CFLAGS += $(shell pkg-config --cflags freetype2)
LDFLAGS = $(shell pkg-config --libs freetype2)

all: ttf_outline

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

%: %.o
	$(CXX) $(LDFLAGS) $^ -o $@

ttf_outline: ttf_outline.o 2d_pack.o

clean:
	rm -f *.o ttf_outline

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean
