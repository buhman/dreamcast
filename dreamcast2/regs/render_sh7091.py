import sys

from generate import renderer
from csv_input import read_input
from block_regs import new_writer
from block_regs import headers
from block_regs import size_to_type

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

def namespace(namespace_name, rows):
    yield f"namespace {namespace_name} {{"
    yield from process(rows)
    yield from blocks(rows)
    yield "}"

if __name__ == "__main__":
    input_file = sys.argv[1]
    namespace_name = sys.argv[2]
    rows = read_input(input_file)
    process = new_writer()
    render, out = renderer()
    render(headers())
    render(namespace(namespace_name, rows))
    sys.stdout.write(out.getvalue())
