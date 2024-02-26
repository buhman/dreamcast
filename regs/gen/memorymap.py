import sys

from csv_input import read_input

from generate import renderer

def includes():
    yield "#include <cstdint>"
    yield ""

def process_rows(rows):
    for row in rows:
        name = row["name"].strip()
        if not name:
            continue
        start = int(row["start"].strip(), 16)
        size = row["size"].strip()
        assert size.endswith("MB"), size
        size = int(size.rstrip("MB"))
        yield name, start, size * 1024 * 1024

def header(processed):
    for name, _, size in processed:
        yield f"extern volatile uint32_t {name}[0x{size:x}] __asm(\"{name}\");"

def lds(processed):
    for name, address, size in processed:
        if address < 0x1000_0000:
            address = address | 0xa000_0000
        yield f"{name} = 0x{address:08x};"

input_file = sys.argv[1]
rows = read_input(input_file)
processed = list(process_rows(rows))
render, out = renderer()
render(includes())
render(header(processed))
sys.stdout.write(out.getvalue())

render, out = renderer()
render(lds(processed))
sys.stderr.write(out.getvalue())
