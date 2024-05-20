import sys

from csv_input import read_input
from generate import renderer

def render_row(row):
    usage = row['usage']
    code = int(row['code'], 16)
    yield f"constexpr uint32_t {usage} = {hex(code)};"

def render_rows(rows):
    yield "#include <cstdint>"
    yield ""
    yield "namespace ft6 {"
    yield "namespace scan_codes {"
    for row in rows:
        yield from render_row(row)
    yield "}"
    yield "}"

if __name__ == "__main__":
    rows = read_input(sys.argv[1])
    render, out = renderer()
    render(render_rows(rows))
    print(out.getvalue())
