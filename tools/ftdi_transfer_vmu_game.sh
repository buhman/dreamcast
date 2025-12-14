#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

filename="$1"

if [ -z "$filename" ]; then
    echo "usage: ./$0 [vmu_image.vms]"
    exit 1
fi

maple_storage="${SCRIPT_DIR}/../example/maple_storage.bin"
image="${filename}_image.bin"

set -ex

make -C "${SCRIPT_DIR}/.." example/maple_storage.bin

python tools/maple_build_image.py "$filename" "$image"

"${SCRIPT_DIR}/ftdi_transfer" \
    write 0xac100000 "$image" \
    write 0xac010000 "$maple_storage" \
    jump  0xac010000 \
    console
