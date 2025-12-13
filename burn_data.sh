#!/bin/sh

filename="$1"

if [ -z "$filename" ]; then
    echo "usage: ./$0 [filename]"
    exit 1
fi

set -ex

cdrecord -speed=8 -v dev=/dev/sr0 -tao -multi -xa tdata01.iso
cdrecord -eject -overburn -speed=8 -v dev=/dev/sr0 -tao -xa "$filename"
