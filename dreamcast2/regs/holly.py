import sys

from csv_input import read_input
from generate import renderer
from block_regs import new_writer
from block_regs import headers

def block():
    yield 'extern struct holly_reg holly __asm("holly");'

input_file = sys.argv[1]
rows = read_input(input_file)
process = new_writer()
render, out = renderer()
render(headers())
render(process(rows))
render(block())
sys.stdout.write(out.getvalue())
