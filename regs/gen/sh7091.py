import csv
import sys
from functools import partial
from pprint import pprint

from generate import renderer

def as_dict(header, row0):
    row = [s.strip() for s in row0]
    return dict(zip(header, row))

def read_input(filename):
    with open(filename) as f:
        reader = csv.reader(f, delimiter=",", quotechar='"')
        header, *rows = reader

    rows = [
        as_dict(header, row)
        for row in rows
        if "".join(map(str, row)).strip()
    ]

    return rows

def size_p(size):
    return size in {1, 2, 4}

def size_to_type(size):
    if size == 1:
        return "reg8 "
    elif size == 2:
        return "reg16"
    elif size == 4:
        return "reg32"
    else:
        raise NotImplemented(size)

def new_writer():
    first_address = 0
    last_address = 0
    last_block = None
    size_total = 0
    reserved_num = 0
    stack = []

    def terminate():
        nonlocal last_block
        nonlocal first_address
        nonlocal stack

        if last_block is not None:
            yield "};"
            for address, name in stack:
                relative_address = address - first_address
                yield f"static_assert((offsetof (struct {last_block.lower()}_reg, {name})) == {hex(relative_address)});"
            yield ""
        stack = []

    def process_row(row):
        nonlocal first_address
        nonlocal last_address
        nonlocal last_block
        nonlocal reserved_num
        nonlocal size_total
        nonlocal stack

        block = row["block"]
        _offset = int(row["offset"], 16) if "offset" in row else 0
        _address = int(row["address"], 16)
        assert _offset  <= 0xff
        assert _address <= 0xffff
        address = (_offset << 16) | (_address << 0)
        size = int(row["size"], 10)
        name = row["name"]
        description = row["description"]

        if block != last_block:
            yield from terminate()
            first_address = address
            last_address = address
            size_total = 0
            reserved_num = 0
            yield f"struct {block.lower()}_reg {{"

        if address != last_address:
            padding = address - last_address
            assert padding > 0, (row, address, last_address)
            type = size_to_type(1)
            yield f"{type} _pad{reserved_num}[{padding}];"
            reserved_num += 1
            size_total += padding

        def field():
            if size_p(size):
                assert address % size == 0
                type = size_to_type(size)
                return f"{type} {name};"
            else:
                type = size_to_type(1)
                return f"{type} {name}[{size}];"

        yield field().ljust(25) + f"/* {description} */"

        stack.append((address, name))
        last_address = address + size
        last_block = block
        size_total += size

    def process(rows):
        for row in rows:
            yield from process_row(row)
        yield from terminate()

    return process

def blocks(rows):
    stack = []
    last_block = None
    for row in rows:
        block = row["block"]
        if last_block != block:
            offset = int(row["offset"], 16)
            stack.append((block, offset))
        last_block = block

    yield "struct sh7091_reg {"
    last_offset = 0
    last_block = None
    reserved_num = 0
    for block, offset in stack:
        if offset != last_offset:
            assert last_block is not None
            type = size_to_type(1)
            raw_pad = (offset - last_offset) << 16
            yield f"{type} _pad{reserved_num}[{hex(raw_pad)} - (sizeof (struct {last_block.lower()}_reg))];"
            reserved_num += 1
        yield f"struct {block.lower()}_reg {block};"
        last_offset = offset
        last_block = block
    yield "};"

    for block, offset in stack:
        yield f"static_assert((offsetof (struct sh7091_reg, {block})) == {hex(offset << 16)});"

    yield ""
    yield 'extern struct sh7091_reg sh7091 __asm("sh7091");'

def headers():
    yield "#include <stdint.h>"
    yield "#include <stddef.h>"
    yield ""
    yield '#include "type.h"'
    yield ""

if __name__ == "__main__":
    input_file = sys.argv[1]
    rows = read_input(input_file)
    process = new_writer()
    render, out = renderer()
    render(headers())
    render(process(rows))
    render(blocks(rows))
    sys.stdout.write(out.getvalue())
