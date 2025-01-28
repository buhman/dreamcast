all: $(patsubst %.cpp,%.bin,$(wildcard example/*.cpp))

include base.mk
include common.mk
include headers.mk

OPT = -Og
MAKEFILE_PATH := $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
CFLAGS += -I$(MAKEFILE_PATH)
LIB ?= $(MAKEFILE_PATH)

include ip.mk

include example/example.mk
include chess/chess.mk
include text_editor/text_editor.mk

.PHONY: phony
