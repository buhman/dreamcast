MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
DIR := $(dir $(MAKEFILE_PATH))

LIB ?= .
OPT ?= -O3
GENERATED ?=

AARCH = -march=armv4 -mlittle-endian

CARCH = -mno-thumb-interwork -march=armv4 -mlittle-endian
#-mtune=arm7di
CFLAGS += -I$(dir $(MAKEFILE_PATH))/../..

CXXFLAGS += -std=c++2a

OBJARCH = -O elf32-littlearm -B armv4

TARGET = arm-none-eabi-

include base.mk

channel.elf: LDSCRIPT = main.lds
channel.elf: start.o channel.o audio.pcm.o

sh4_interrupt.elf: LDSCRIPT = main.lds
sh4_interrupt.elf: start.o sh4_interrupt.o

xm.elf: LDSCRIPT = main.lds
xm.elf: start.o xm.o audio.pcm.o

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(o|d|bin|elf|gch)$$' \
		-exec rm {} \;

.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
