CFLAGS += -O3
CFLAGS += -std=c++23
CFLAGS += -Wl,-z,stack-size=314572800
CFLAGS += -Wall -Wno-deprecated -Werror -Wfatal-errors
CFLAGS += -g -gdwarf-4
#CFLAGS += -I../..

k_means_vq: k_means_vq.cpp ppm.c
	g++ $(CFLAGS) $^ -o $@
