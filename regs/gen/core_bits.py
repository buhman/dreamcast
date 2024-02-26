import sys
from collections import defaultdict
from pprint import pprint
from dataclasses import dataclass
from typing import Union

from csv_input import read_input
from generate import renderer

def aggregate_registers(d):
    aggregated = defaultdict(list)
    for row in d:
        #assert row["register_name"] != ""
        aggregated[row["register_name"]].append(row)
    return dict(aggregated)

def parse_bit_number(s):
    assert '-' not in s
    return int(s, 10)

def parse_bit_set(s, split_char):
    assert len(list(c for c in s if c == split_char)) == 1
    left, right = map(parse_bit_number, s.split(split_char, maxsplit=1))
    assert left > right, (left, right)
    return left, right

def parse_bit_range(s):
    if '-' in s:
        left, right = parse_bit_set(s, '-')
        return set(range(right, left+1))
    elif ',' in s:
        left, right = parse_bit_set(s, ',')
        return set([right, left])
    else:
        num = parse_bit_number(s)
        return set([num])

def aggregate_enums(aggregated_rows):
    non_enum = []
    enum_aggregated = defaultdict(list)
    all_bits = set()
    enum_bits = dict()

    def assert_unique_ordered(bits, row):
        nonlocal all_bits
        assert all(bit not in all_bits for bit in bits), (bits, row)
        assert max(all_bits, default=32) > max(bits), (all_bits, bits)
        all_bits |= bits

    for row in aggregated_rows:
        bits = parse_bit_range(row["bits"])
        assert row["bit_name"] != "", row
        if row["enum_name"] == "":
            assert_unique_ordered(bits, row)
            non_enum.append(row)
        else:
            if row["enum_name"] not in enum_bits:
                assert_unique_ordered(bits, row)
                non_enum.append(row["enum_name"])
            else:
                assert enum_bits[row["enum_name"]] == bits, row

            enum_bits[row["enum_name"]] = bits
            enum_aggregated[row["enum_name"]].append(row)

    return non_enum, dict(enum_aggregated)

@dataclass
class enum:
    name: str
    defs: list[dict]

@dataclass
class register:
    block: Union[None, str]
    name: str
    defs: list[Union[dict, enum]]

def aggregate_all_enums(aggregated):
    out = []
    for register_name, rows in aggregated.items():
        non_enum, enum_aggregated = aggregate_enums(rows)
        def resolve(row_or_string):
            if type(row_or_string) == str:
                return enum(row_or_string,
                            enum_aggregated[row_or_string])
            elif type(row_or_string) == dict:
                return row_or_string
            else:
                assert False, (row_or_string, type(row_or_string))

        defs = [resolve(aggregate) for aggregate in non_enum]
        if 'block' in rows[0]:
            blocks = set(row['block'] for row in rows)
            assert len(blocks) == 1, blocks
            block_name = next(iter(blocks))
            out.append(
                register(block_name, register_name, defs))
        else:
            out.append(
                register(None, register_name, defs))
    return out

'''
 register(name='SCALER_CTL',
          defs=[enum(name='field_select',
                     defs=[{'bit_name': 'field_1',
                            'bits': '18',
                            'description': '',
                            'enum_name': 'field_select',
                            'mask': '',
                            'register_name': 'SCALER_CTL',
                            'value': '0'},
                           {...}]),
                {'bit_name': 'interlace',
                 'bits': '17',
                 'description': '',
                 'enum_name': '',
                 'mask': '',
                 'register_name': 'SCALER_CTL',
                 'value': '1'},
                {...},
                ...]),
'''

def mask_from_bits(bits):
    h, l = max(bits), min(bits)
    mask = 2 ** ((h - l) + 1) - 1
    return mask

def parse_value(value):
    return eval(value)

def escape(bit_name):
    if bit_name[0] in {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}:
        return '_' + bit_name
    else:
        return bit_name

def render_read_only(bit_def):
    assert bit_def["value"] == ""
    assert bit_def["mask"] == ""
    bits = parse_bit_range(bit_def["bits"])
    mask_value = mask_from_bits(bits)
    yield (
        f"constexpr uint32_t {escape(bit_def['bit_name'])}(uint32_t reg) {{ "
        f"return (reg >> {min(bits)}) & {hex(mask_value)};"
        " }"
    )

def render_float_mask(mask):
    if mask == "float_0_8_23":
        return "_i(__builtin_fabsf(num));"
    elif mask == "float_1_8_23":
        return "_i(num)"
    else:
        assert mask.startswith("float_")
        mask = mask.removeprefix("float_")
        sign, exponent, fraction = map(int, mask.split('_'))
        assert exponent == 8, exponent
        assert sign == 1
        bit_length = (sign + exponent + fraction)
        mask = (2 ** bit_length - 1) << (32 - bit_length)
        return f"_i(num) & {hex(mask)}"

def render_mask(bit_def):
    assert bit_def["value"] == ""
    mask = bit_def["mask"]
    bits = parse_bit_range(bit_def["bits"])
    if mask.startswith("float_"):
        yield (
            f"inline uint32_t {escape(bit_def['bit_name'])}(float num) {{ "
            f"return {render_float_mask(mask)};"
            " }"
        )
    else:
        assert mask.startswith("0x") or mask.startswith("0b") or mask[0] in set(range(0, 9+1)), mask
        mask_value = eval(mask)
        assert mask_value & mask_from_bits(bits) == mask_value, (mask_value, mask_from_bits(bits))

        yield (
            f"constexpr uint32_t {escape(bit_def['bit_name'])}(uint32_t num) {{ "
            f"return (num & {hex(mask_value)}) << {min(bits)};"
            " }"
        )

def render_value(bit_def):
    assert bit_def["mask"] == ""
    bits = parse_bit_range(bit_def["bits"])
    assert parse_value(bit_def["value"]) <= mask_from_bits(bits), (bit_def["value"], mask_from_bits(bits), bits)
    bit_ix = min(bits)
    yield f"constexpr uint32_t {escape(bit_def['bit_name'])} = {bit_def['value']} << {bit_ix};"

def render_defs(bit_def):
    if bit_def["value"] != "":
        yield from render_value(bit_def)
    elif bit_def["mask"] != "":
        yield from render_mask(bit_def)
    else:
        yield from render_read_only(bit_def)

def render_enum_mask(enum_def):
    all_bits = set(bit_def["bits"] for bit_def in enum_def.defs)
    assert len(all_bits) == 1
    assert all(bit_def["bit_name"] != "bit_mask" for bit_def in enum_def.defs), bit_def
    _bits = next(iter(all_bits))
    bits = parse_bit_range(_bits)
    mask_value = mask_from_bits(bits)
    bit_ix = min(bits)
    yield ""
    yield f"constexpr uint32_t bit_mask = {hex(mask_value)} << {bit_ix};"

def render_enum(enum_def):
    yield f"namespace {enum_def.name.lower()} {{"
    for bit_def in enum_def.defs:
        yield from render_defs(bit_def)
    yield from render_enum_mask(enum_def)
    yield "}"

def render_register(register):
    if register.name != "":
        yield f"namespace {register.name.lower()} {{"

    last = None
    for ix, bit_def in enumerate(register.defs):
        if type(bit_def) is enum:
            if ix != 0:
                yield ""
            yield from render_enum(bit_def)
        else:
            if ix != 0 and type(last) is enum:
                yield ""
            yield from render_defs(bit_def)
        last = bit_def

    if register.name != "":
        yield "}"
    yield ""

def render_registers(registers, file_namespace):
    if file_namespace is not None:
        yield f"namespace {file_namespace} {{"

    last_block = None
    for register in registers:
        if register.block != last_block:
            assert register.block is not None
            if last_block is not None:
                yield '}' # end of previous namespace
                yield ""
            yield f'namespace {register.block.lower()} {{'
        if register.block is None:
            assert last_block is None
        last_block = register.block

        yield from render_register(register)

    if last_block is not None:
        yield '}' # end of block namespace

    if file_namespace is not None:
        yield '}' # end of file namespace

def header():
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield ""
    yield '#include "../float_uint32.hpp"'
    yield ""

if __name__ == "__main__":
    d = read_input(sys.argv[1])
    file_namespace = sys.argv[2] if len(sys.argv) > 2 else None
    aggregated = aggregate_registers(d)
    registers = aggregate_all_enums(aggregated)
    render, out = renderer()
    render(header())
    render(render_registers(registers, file_namespace))
    sys.stdout.write(out.getvalue())
