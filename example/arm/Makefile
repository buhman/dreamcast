MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
DIR := $(dir $(MAKEFILE_PATH))

LIB ?= .
OPT ?= -Og
GENERATED ?=

AARCH = -march=armv4 -mlittle-endian

CARCH = -mno-thumb-interwork -march=armv4 -mtune=arm7di -mlittle-endian
CFLAGS += -I$(dir $(MAKEFILE_PATH))/../..

CXXFLAGS += -std=c++2a

OBJARCH = -O elf32-littlearm -B armv4

TARGET = arm-none-eabi-

include base.mk

channel.elf: LDSCRIPT = main.lds
channel.elf: start.o channel.o audio.pcm.o

sh4_interrupt.elf: LDSCRIPT = main.lds
sh4_interrupt.elf: start.o sh4_interrupt.o

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
