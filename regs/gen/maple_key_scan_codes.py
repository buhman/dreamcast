import sys
import string

from csv_input import read_input
from generate import renderer

def render_row(row):
    usage = row['usage']
    code = int(row['code'], 16)
    yield f"constexpr uint32_t {usage} = {hex(code)};"

def render_rows(rows):
    yield "namespace ft6 {"
    yield "namespace scan_code {"
    for row in rows:
        yield from render_row(row)
    yield "}"
    yield "}"

def code_point(s):
    if not s.strip():
        return '0'
    elif len(s) == 1:
        assert s in string.printable
        if s == '\\':
            return "'\\\\'"
        elif s.strip() == "'":
            return "'\\''"
        else:
            return f"'{s}'"
    else:
        assert False, s

last_printable = 0x38

def render_normal_shift(row):
    usage = row['usage']
    normal = code_point(row['normal'])
    shift = code_point(row['shift'])
    yield f"[scan_code::{usage}] = {{ {normal}, {shift} }},"

def render_scancode_code_point(rows):
    yield "namespace ft6 {"
    yield "namespace scan_code {"
    yield f"constexpr uint32_t last_printable = {hex(last_printable)};"
    yield ""
    yield f"const uint8_t code_point[last_printable + 1][2] = {{"
    for i, row in enumerate(rows):
        yield from render_normal_shift(row)
        if i == last_printable:
            break
    yield "};"
    yield "}"
    yield "}"

def header():
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield ""

if __name__ == "__main__":
    rows = read_input(sys.argv[1])
    render, out = renderer()
    render(header())
    render(render_rows(rows))
    render(render_scancode_code_point(rows))
    print(out.getvalue())
