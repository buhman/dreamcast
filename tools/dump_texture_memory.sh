#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

set -ex

${SCRIPT_DIR}/ftdi_transfer \
    read 0xa5000000 0x800000 ./texture_memory.bin \
    read 0xa05f8000 0x2000 ./holly_registers.bin
