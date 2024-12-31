import sys
from os import path

from csv_input import read_input
from generate import renderer

from byte_position import parse

type_dict = {
    1: 'uint8_t',
    2: 'uint16_t',
    4: 'uint16_le_be',
    8: 'uint32_le_be',
}

def header():
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield "#include <cstddef>"
    yield ""
    yield '#include "uint_le_be.hpp"'
    yield ""

def render_fields(input_name, fields):
    yield "namespace iso9660 {"

    yield f"struct {input_name} {{"
    for field in fields:
        field_size = (field.end - field.start) + 1
        if field.content == 'numerical_value':
            assert field_size in type_dict, field
            type = type_dict[field_size]
            yield f"const {type} {field.name};"
        else:
            if field_size == 1:
                yield f"const uint8_t {field.name};"
            elif field_size == 0:
                yield f"const uint8_t {field.name}[];"
            else:
                yield f"const uint8_t {field.name}[{field_size}];"
    yield "};"

    for field in fields:
        yield f"static_assert((offsetof (struct {input_name}, {field.name})) == {field.start - 1});"

    yield "}" # namespace

if __name__ == "__main__":
    input_file = sys.argv[1]
    input_name0, _ = path.splitext(input_file)
    _, input_name = path.split(input_name0)
    rows = read_input(input_file)
    fields = list(parse(rows))
    render, out = renderer()
    render(header())
    render(render_fields(input_name, fields))
    sys.stdout.write(out.getvalue())
