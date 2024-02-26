import sys

from sh7091 import new_writer
from csv_input import read_input
from sh7091 import headers
from sh7091 import blocks
from generate import renderer

def blocks(rows):
    blocks = []
    for row in rows:
        block = row["block"]
        if block not in blocks:
            blocks.append(block)

    for block in blocks:
        yield f'extern struct {block.lower()}_reg {block.lower()} __asm("{block.lower()}");'

input_file = sys.argv[1]
rows = read_input(input_file)
process = new_writer()
render, out = renderer()
render(headers())
render(process(rows))
render(blocks(rows))
sys.stdout.write(out.getvalue())
