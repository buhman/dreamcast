import sys
from dataclasses import dataclass
from collections import defaultdict

from csv_input import read_input_headerless
from generate import renderer

@dataclass
class Bit:
    name: str
    length: int
    position: int

@dataclass
class Field:
    name: str
    bits: list[str]

@dataclass
class Format:
    name: str
    fields: list[Field]
    field_order: list[str]
    size: int

def parse_bits(bits: list[str]):
    bit_order = [7, 6, 5, 4, 3, 2, 1, 0]
    by_name = defaultdict(list)
    for bit_ix, bit in zip(bit_order, bits):
        by_name[bit].append(bit_ix)
    for name, indicies in by_name.items():
        yield Bit(name=name,
                  length=len(indicies),
                  position=min(indicies),
                  )

def parse_data_format(ix, rows):
    if ix >= len(rows):
        return None

    while rows[ix][0] == "":
        ix += 1
        if ix >= len(rows):
            return None

    format_name, *header = rows[ix]
    ix += 1
    assert format_name != ""
    assert header == ["7", "6", "5", "4", "3", "2", "1", "0"]

    fields = defaultdict(list)
    field_order = list()
    size = 0
    while ix < len(rows) and rows[ix][0] != "":
        field_name, *_bits = rows[ix]
        ix += 1
        excess_bits = [b for b in _bits[8:] if b != ""]
        assert excess_bits == []
        bits = [b for b in _bits[:8] if b != ""]
        assert len(bits) in {0, 8}, bits
        fields[field_name].append(Field(field_name,
                                        list(parse_bits(bits))))
        size += 1
        if field_name not in field_order:
            field_order.append(field_name)

    return ix, Format(format_name, dict(fields), field_order, size)

def parse(rows):
    ix = 0
    formats = []

    while True:
        ix_format = parse_data_format(ix, rows)
        if ix_format is None:
            break
        ix, format = ix_format
        formats.append(format)

    assert len(formats) > 0
    return formats

def render_format(format):
    yield f"namespace {format.name} {{"
    for field_name in format.field_order:
        subfields = format.fields[field_name]
        if not any(field.bits != [] for field in subfields):
            continue

        yield f"namespace {field_name} {{"
        for ix, field in enumerate(subfields):
            bit_offset = 8 * ix
            for bit in field.bits:
                name = bit.name.lower()
                pos = bit_offset + bit.position
                mask = bin(2 ** bit.length - 1)
                yield f"constexpr uint32_t {name}() {{ return {mask} << {pos}; }}"
                yield f"constexpr uint32_t {name}(uint32_t reg) {{ return (reg >> {pos}) & {mask}; }}"
                yield ""
        yield "}"
        yield ""

    yield f"struct data_format {{"

    for field_name in format.field_order:
        subfields = format.fields[field_name]
        if len(subfields) == 1:
            field, = subfields
            yield f"uint8_t {field_name};"
        elif len(subfields) == 2:
            yield f"uint16_t {field_name};"
        elif len(subfields) == 6:
            yield f"uint8_t {field_name}[6];"
        else:
            assert False, (len(subfields), field_name)

    yield "};"
    assert format.size % 4 == 0, format.size
    yield f"static_assert((sizeof (struct data_format)) == {format.size});"
    yield "}"

def render_formats(name, formats):
    yield "#pragma once"

    yield f"namespace {name} {{"
    for format in formats:
        yield from render_format(format)
        yield ""

    yield "}"

if __name__ == "__main__":
    rows = read_input_headerless(sys.argv[1])
    name = sys.argv[1].split('.')[0].split('_')[-1]
    assert len(name) == 3 or len(name) == 4
    formats = parse(rows)
    render, out = renderer()
    render(render_formats(name, formats))
    print(out.getvalue())
