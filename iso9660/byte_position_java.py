import sys
from os import path

from csv_input import read_input
from generate import renderer

from byte_position import parse

size_to_accessor = {
    1: "getByte",
    4: "getShortLE",
    8: "getIntLE",
}

def _camelcase(name, first_upper):
    want_upper = first_upper
    for c in name:
        if c == '_':
            want_upper = True
        else:
            yield c.upper() if want_upper else c
            want_upper = False

def pascalcase(name):
    return "".join(_camelcase(name, first_upper=True))

def camelcase(name):
    return "".join(_camelcase(name, first_upper=False))

def render_fields(input_name, fields):
    yield "package filesystem.iso9660;"


    yield f"public class {pascalcase(input_name)} extends ByteParser {{"
    for field in fields:
        if "_res" in field.name.lower():
            continue

        yield f"public static final int {field.name.upper()}_START = {field.start - 1};"
        yield f"public static final int {field.name.upper()}_END = {field.end - 1};"

    yield f"{pascalcase(input_name)}(byte[] array, int offset) {{"
    yield "super(array, offset);"
    yield "}"

    for field in fields:
        if "_res" in field.name.lower():
            continue

        if field.content == 'numerical_value':
            field_size = (field.end - field.start) + 1
            accessor = size_to_accessor[field_size]

            yield f"public int {camelcase(field.name)}() {{"
            yield f"return {accessor}({field.name.upper()}_START);"
            yield "}"

    yield "}"

if __name__ == "__main__":
    input_file = sys.argv[1]
    input_name0, _ = path.splitext(input_file)
    _, input_name = path.split(input_name0)
    rows = read_input(input_file)
    fields = list(parse(rows))
    render, out = renderer()
    render(render_fields(input_name, fields))
    sys.stdout.write(out.getvalue())
