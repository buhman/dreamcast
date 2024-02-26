import sys
from dataclasses import dataclass
from os import path

from csv_input import read_input
from generate import renderer

from pprint import pprint

def parse_bp(s):
    if ' to ' in s:
        start0, end0 = s.split(' to ')
        if '(' in end0 and ')' in end0 and 'LEN_' in end0:
            end0 = int(start0) - 1
            start, end = int(start0), int(end0)
            return start, end
        else:
            start, end = int(start0), int(end0)
            assert start <= end, (start, end)
            return start, end
    else:
        start = int(s)
        return start, start

def bp_range(start, end):
    return set(range(start, end+1))

reserved = 0
def sanitize_field_name(name):
    global reserved
    if name == "(Reserved for future standardization)" or name == "Unused field":
        reserved += 1
        return f"_res{reserved}";
    if '(' in name:
        assert 'LEN_' in name, name
        name = name.split('(')[0].strip()

    name = name.lower().replace(' ', '_')
    return name

def sanitize_content_name(name):
    if name == 'Numerical value':
        return 'numerical_value'
    else:
        return 'bytes'

@dataclass
class Field:
    start: int
    end: int
    name: str
    content: str

def parse(rows):
    seen_bps = set()
    seen_names = set()

    for row in rows:
        start, end = parse_bp(row['BP'])
        _range = bp_range(start, end)
        assert seen_bps.intersection(_range) == set(), row
        seen_bps = seen_bps.union(_range)
        field_name = sanitize_field_name(row["Field name"])
        assert field_name not in seen_names
        seen_names.add(field_name)
        content_name = sanitize_content_name(row["Content"])

        yield Field(
            start=start,
            end=end,
            name=field_name,
            content=content_name
        )

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
