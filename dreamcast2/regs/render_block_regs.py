import sys

from csv_input import read_input
from generate import renderer
from block_regs import new_writer
from block_regs import headers

def blocks(rows):
    blocks = []
    for row in rows:
        block = row["block"]
        if block not in blocks:
            blocks.append(block)

    for block in blocks:
        yield f'extern struct {block.lower()}_reg {block.lower()} __asm("{block.lower()}");'

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
