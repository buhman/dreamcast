import sys

from sh7091 import new_writer
from sh7091 import read_input
from sh7091 import headers
from generate import renderer

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
