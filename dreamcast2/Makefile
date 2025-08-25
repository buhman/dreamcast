include base.mk

OPT = -O2

MAKEFILE_PATH := $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
CFLAGS += -I$(MAKEFILE_PATH)
LIB ?= $(MAKEFILE_PATH)

include example/example.mk

clean:
	find -P \
		-regextype posix-egrep \
		-regex '.*\.(iso|o|d|bin|elf|cue|gch)$$' \
		-exec rm {} \;
