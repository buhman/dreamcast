import sys

from generate import renderer
from csv_input import read_input

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
    next_address = 0
    last_block = None
    size_total = 0
    reserved_num = 0
    stack = []

    def terminate():
        nonlocal last_block
        nonlocal stack

        if last_block is not None:
            yield "};"
            for address, name in stack:
                yield f"static_assert((offsetof (struct {last_block.lower()}_reg, {name})) == {hex(address - first_address)});"
            yield ""
        stack = []

    def process_row(row):
        nonlocal first_address
        nonlocal next_address
        nonlocal last_block
        nonlocal reserved_num
        nonlocal size_total
        nonlocal stack

        block = row["block"]
        _offset = int(row["offset"], 16) if "offset" in row else 0
        _address = int(row["address"], 16)
        assert _offset  <= 0xff
        assert _address <= 0xffff
        offset_address = (_offset << 16)
        address = offset_address | (_address << 0)
        size = int(row["size"], 10)
        name = row["name"]
        description = row["description"]

        if block != last_block:
            yield from terminate()
            first_address = offset_address
            next_address = offset_address
            size_total = 0
            reserved_num = 0
            yield f"struct {block.lower()}_reg {{"

        assert address >= next_address, row
        if address > next_address:
            padding = address - next_address
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
                type = size_to_type(4)
                return f"{type} {name}[{size // 4}];"

        yield field().ljust(27) + f"/* {description} */"

        stack.append((address, name))
        next_address = address + size
        last_block = block
        size_total += size

    def process(rows):
        for row in rows:
            yield from process_row(row)
        yield from terminate()

    return process

def headers():
    yield "#pragma once"
    yield ""
    yield '#include "reg.hpp"'
    yield ""
