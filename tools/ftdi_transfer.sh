#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

filename="$1"

if [ -z "$filename" ]; then
    echo "usage: ./$0 [filename]"
    exit 1
fi

set -ex

${SCRIPT_DIR}/ftdi_transfer \
    write 0xac010000 "$filename" \
    jump  0xac010000
#    console
