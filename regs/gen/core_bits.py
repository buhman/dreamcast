import sys
from collections import defaultdict
from pprint import pprint
from dataclasses import dataclass
from typing import Union

from sh7091 import read_input
from generate import renderer

def aggregate_registers(d):
    aggregated = defaultdict(list)
    for row in d:
        assert row["register_name"] != ""
        aggregated[row["register_name"]].append(row)
    return dict(aggregated)

def parse_bit_number(s):
    assert '-' not in s
    return int(s, 10)

def parse_bit_range(s):
    if '-' in s:
        left, right = map(parse_bit_number, s.split('-', maxsplit=1))
        assert left > right, (left, right)
        return set(range(right, left+1))
    else:
        num = parse_bit_number(s)
        return set([num])

def aggregate_enums(aggregated_rows):
    non_enum = []
    enum_aggregated = defaultdict(list)
    all_bits = set()
    enum_bits = dict()

    def assert_unique_ordered(bits):
        nonlocal all_bits
        assert all(bit not in all_bits for bit in bits), bits
        assert max(all_bits, default=32) > max(bits), (all_bits, bits)
        all_bits |= bits

    for row in aggregated_rows:
        bits = parse_bit_range(row["bits"])
        assert row["bit_name"] != ""
        if row["enum_name"] == "":
            assert_unique_ordered(bits)
            non_enum.append(row)
        else:
            if row["enum_name"] not in enum_bits:
                assert_unique_ordered(bits)
                non_enum.append(row["enum_name"])
            else:
                assert enum_bits[row["enum_name"]] == bits

            enum_bits[row["enum_name"]] = bits
            enum_aggregated[row["enum_name"]].append(row)

    return non_enum, dict(enum_aggregated)

@dataclass
class enum:
    name: str
    defs: list[dict]

@dataclass
class register:
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
        out.append(
            register(register_name,
                     [resolve(aggregate) for aggregate in non_enum]))
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
    mask = 2 ** len(bits) - 1
    return mask

def parse_value(value):
    return eval(value)

def render_read_only(bit_def):
    assert bit_def["value"] == ""
    assert bit_def["mask"] == ""
    bits = parse_bit_range(bit_def["bits"])
    mask_value = mask_from_bits(bits)
    yield (
        f"constexpr uint32_t {bit_def['bit_name']}(uint32_t reg) {{ "
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
            f"constexpr uint32_t {bit_def['bit_name']}(float num) {{ "
            f"return {render_float_mask(mask)};"
            " }"
        )
    else:
        assert mask.startswith("0x") or mask.startswith("0b") or mask[0] in set(range(0, 9+1)), mask
        mask_value = eval(mask)
        assert mask_value & mask_from_bits(bits) == mask_value, (mask_value, mask_from_bits(bits))

        yield (
            f"constexpr uint32_t {bit_def['bit_name']}(uint32_t num) {{ "
            f"return (num & {hex(mask_value)}) << {min(bits)};"
            " }"
        )

def render_value(bit_def):
    assert bit_def["mask"] == ""
    bits = parse_bit_range(bit_def["bits"])
    assert parse_value(bit_def["value"]) <= mask_from_bits(bits), bit_def["value"]
    bit_ix = min(bits)
    yield f"constexpr uint32_t {bit_def['bit_name']} = {bit_def['value']} << {bit_ix};"

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

    yield "}"
    yield ""

def render_registers(registers):
    for register in registers:
        yield from render_register(register)

def header():
    yield "#include <cstdint>"
    yield ""
    yield '#include "../float_uint32.hpp"'
    yield ""

if __name__ == "__main__":
    d = read_input(sys.argv[1])
    aggregated = aggregate_registers(d)
    registers = aggregate_all_enums(aggregated)
    render, out = renderer()
    render(header())
    render(render_registers(registers))
    sys.stdout.write(out.getvalue())
