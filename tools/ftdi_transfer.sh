#!/bin/sh

filename="$1"

if [ -z "$filename" ]; then
    echo "usage: ./$0 [filename]"
    exit 1
fi

set -ex

./ftdi_transfer \
    write 0xac010000 "$filename" \
    jump  0xac010000 \
    console
