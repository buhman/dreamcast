include base.mk

OPT = -O2

MAKEFILE_PATH := $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
CFLAGS += -I$(MAKEFILE_PATH)
LIB ?= $(MAKEFILE_PATH)

include example/example.mk
