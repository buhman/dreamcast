import sys

from sh7091 import new_writer
from sh7091 import read_input
from sh7091 import headers
from sh7091 import blocks
from generate import renderer

input_file = sys.argv[1]
rows = read_input(input_file)
process = new_writer()
render, out = renderer()
render(headers())
render(process(rows))
sys.stdout.write(out.getvalue())
