from dataclasses import dataclass
from pprint import pprint
import sys
import csv

from generate import renderer

_field_types = {
    "parameter_control_word": "uint32_t",
    "user_clip_": "uint32_t",
    "object_pointer": "uint32_t",
    "bounding_box_": "uint32_t",
    "isp_tsp_instruction_word": "uint32_t",
    "tsp_instruction_word": "uint32_t",
    "texture_control_word": "uint32_t",
    "data_size_for_sort_dma": "uint32_t",
    "next_address_for_sort_dma": "uint32_t",
    "face_color_": "float",
    "face_offset_color_": "float",
    "x": "float",
    "y": "float",
    "z": "float",
    "base_color_": "float",
    "base_color_0": "uint32_t",
    "base_color_1": "uint32_t",
    "offset_color_0": "uint32_t",
    "offset_color_1": "uint32_t",
    "base_intensity_": "uint32_t",
    "u": "float",
    "v": "float",
    "u_v": "uint32_t",
    "base_color": "uint32_t",
    "offset_color": "uint32_t",
    "offset_color_": "float",
    "base_intensity": "float",
    "offset_intensity": "float",
    "a_": "float",
    "b_": "float",
    "c_": "float",
    "d_": "float",
    "a_u_a_v": "uint32_t",
    "b_u_b_v": "uint32_t",
    "c_u_c_v": "uint32_t",
    "_res": "uint32_t"
}

def get_type(field_name: str):
    match = None
    match_len = 0
    for name, type in _field_types.items():
        if field_name.startswith(name) and len(name) >= match_len:
            match = type
            assert match_len != len(name), (name, match)
            match_len = len(name)
    assert match != None, field_name
    return match

class EndOfInput(Exception):
    pass

def next_row(ix, rows, advance):
    if ix >= len(rows):
        raise EndOfInput

    if advance:
        while rows[ix][0] == "":
            ix += 1
            if ix >= len(rows):
                raise EndOfInput
    row = rows[ix]
    ix += 1
    return ix, row

@dataclass
class FieldDeclaration:
    offset: int
    name: str

@dataclass
class StructDeclaration:
    name: str
    fields: list[FieldDeclaration]
    size: int

def parse_type_declaration(ix, rows):
    ix, row = next_row(ix, rows, advance=True)
    assert len(row) == 2, row
    struct_name, empty = row
    assert empty == "", row
    fields = []
    last_offset = -4
    res_ix = 0

    def terminate():
        size = last_offset + 4
        assert size == 32 or size == 64, size
        return ix, StructDeclaration(
            struct_name,
            fields,
            size
        )

    while True:
        try:
            ix, row = next_row(ix, rows, advance=False)
        except EndOfInput:
            return terminate()
        if row[0] == "":
            return terminate()
        else:
            assert len(row) == 2, row
            _offset, name = row
            offset = int(_offset, 16)
            assert offset == last_offset + 4, (hex(offset), hex(last_offset))
            last_offset = offset
            if name == "":
                name = f"_res{res_ix}"
                res_ix += 1
            fields.append(FieldDeclaration(offset, name))

def parse(rows):
    ix = 0
    declarations = []
    while True:
        try:
            ix, declaration = parse_type_declaration(ix, rows)
        except EndOfInput:
            break
        declarations.append(declaration)

    return declarations

def render_initializer(declaration):
    initializer = f"{declaration.name}("
    padding = " " * len(initializer)
    def start(i):
        if i == 0:
            return initializer
        else:
            return padding

    nonres_fields = [f for f in declaration.fields if not f.name.startswith('_res')]
    for i, field in enumerate(nonres_fields):
        s = start(i)
        type = get_type(field.name)
        comma = ',' if i + 1 < len(nonres_fields) else ''
        yield s + f"const {type} {field.name}" + comma
    yield padding + ')'

    for i, field in enumerate(declaration.fields):
        value = field.name if not field.name.startswith('_res') else '0'
        s = ':' if i == 0 else ','
        yield "  " + s + f" {field.name}({value})"
    yield "{ }"


def render_static_assertions(declaration):
    yield f"static_assert((sizeof ({declaration.name})) == {declaration.size});"
    for field in declaration.fields:
        yield f"static_assert((offsetof (struct {declaration.name}, {field.name})) == 0x{field.offset:02x});"

def render_declaration(declaration):
    yield f"struct {declaration.name} {{"
    for field in declaration.fields:
        type = get_type(field.name)
        yield f"{type} {field.name};"
    yield ""
    yield from render_initializer(declaration)
    yield "};"
    yield from render_static_assertions(declaration)

def render_declarations(namespace, declarations):
    yield f"namespace {namespace} {{"
    for declaration in declarations:
        yield from render_declaration(declaration)
        yield ""
    yield "}"

def headers():
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield ""

def read_input(filename):
    with open(filename) as f:
        reader = csv.reader(f, delimiter=",", quotechar='"')
        rows = [
            [s.strip() for s in row]
            for row in reader
        ]
    return rows

if __name__ == "__main__":
    rows = read_input(sys.argv[1])
    namespace = sys.argv[2]
    declarations = parse(rows)
    render, out = renderer()
    render(headers())
    render(render_declarations(namespace, declarations))
    print(out.getvalue())
