################################################################################
# architecture flags
################################################################################

AARCH = --isa=sh4 --little

CARCH ?= -m4-single-only -ml
CFLAGS += -mfsca -mfsrra -funsafe-math-optimizations -ffast-math

OBJARCH = -O elf32-shl -B sh4

TARGET = sh4-none-elf-

################################################################################
# architecture-agnostic flags
################################################################################

DEBUG = -g -gdwarf-4

AFLAGS += --fatal-warnings

CSTD ?= -std=gnu23

CXXSTD ?= -std=c++23

CFLAGS += -ffreestanding -nostdlib -fno-builtin
CFLAGS += -falign-functions=4
CFLAGS += -ffunction-sections -fdata-sections -fshort-enums
CFLAGS += -Wall -Werror -Wfatal-errors -Winline
CFLAGS += -Wno-array-bounds
CFLAGS += -Wno-error=maybe-uninitialized
CFLAGS += -Wno-error=unused-but-set-variable
CFLAGS += -Wno-error=unused-variable
CFLAGS += -Wno-error=unused-function
CFLAGS += -D__dreamcast__

CXXFLAGS += -fno-exceptions -fno-non-call-exceptions -fno-rtti -fno-threadsafe-statics

LDFLAGS += --gc-sections --no-warn-rwx-segment --print-memory-usage --entry=_start --orphan-handling=error

LD_LIBGCC += -L $(dir $(shell $(CC) -print-file-name=libgcc.a)) -lgcc

DEPFLAGS = -MMD -MP

################################################################################
# toolchain
################################################################################

CC = $(TARGET)gcc
CXX = $(TARGET)g++
AS = $(TARGET)as
LD = $(TARGET)ld
OBJCOPY = $(TARGET)objcopy
OBJDUMP = $(TARGET)objdump

################################################################################
# base rules
################################################################################

%.o: %.s
	$(AS) $(AARCH) $(AFLAGS) $(DEBUG) $< -o $@

%.o: %.c
	$(CC) $(CARCH) $(CFLAGS) $(CSTD) $(OPT) $(DEBUG) $(DEPFLAGS) -MF ${<}.d -c $< -o $@

%.o: %.cpp
	$(CXX) $(CARCH) $(CFLAGS) $(CXXSTD) $(CXXFLAGS) $(OPT) $(DEBUG) $(DEPFLAGS) -MF ${<}.d -c $< -o $@

%.elf:
	$(LD) $(LDFLAGS) -L $(LIB) -T $(LDSCRIPT) $^ $(LD_LIBGCC) -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	du -b $@

-include $(shell find -type f -name '*.d')
