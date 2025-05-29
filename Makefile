all: $(patsubst %.cpp,%.bin,$(wildcard example/*.cpp))

include base.mk
include common.mk
include headers.mk

OPT = -O2
MAKEFILE_PATH := $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
CFLAGS += -I$(MAKEFILE_PATH)
LIB ?= $(MAKEFILE_PATH)

include ip.mk

include example/example.mk
include example/bsp/bsp.mk
include example/aica/aica.mk
include chess/chess.mk
include text_editor/text_editor.mk
include assets.mk

.PHONY: phony
