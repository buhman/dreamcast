import sys
from dataclasses import dataclass
from collections import defaultdict

from generate import renderer
from csv_input import read_input

@dataclass
class Part:
    index: int
    address: int
    register_bits: tuple[int, int]
    argument_bits: tuple[int, int]

@dataclass
class Register:
    name: str
    parts: list[Part]

def parse_slice(s):
    if '-' in s:
        start0, end0 = s.split('-')
        start, end = int(start0, 10), int(end0, 10)
        assert start > end, s
        return start, end
    else:
        start = int(s, 10)
        return start, start

def slice_bits(s):
    start, end = s
    return set(range(start, end+1))

def parse_row(row):
    address = int(row["address"], 16)
    name = row["name"]
    part_index0 = row["part"].strip()
    part_index = int(part_index0, 10) if part_index0 else 0

    register_bits = parse_slice(row["register_bits"])
    if row["argument_bits"].strip():
        argument_bits = parse_slice(row["argument_bits"])
        assert len(slice_bits(argument_bits)) == len(slice_bits(register_bits)), row
    else:
        start, end = register_bits
        argument_bits = (start - end, 0)

    part = Part(
        part_index,
        address,
        register_bits,
        argument_bits,
    )
    return name, part

def group_parts(rows, address_increment):
    by_name: dict[str, Register] = {}
    register_bit_alloc: dict[int, set[int]] = defaultdict(set)
    argument_bit_alloc: dict[str, set[int]] = defaultdict(set)
    register_order = []
    addresses = []
    last_address = 0

    for row in rows:
        name, part = parse_row(row)
        assert part.address >= last_address, row
        assert part.address % address_increment == 0, row
        last_address = part.address
        if part.address not in set(a[0] for a in addresses):
            addresses.append((part.address, [name]))
        else:
            assert addresses[-1][0] == part.address
            addresses[-1][1].append(name)

        bits = slice_bits(part.register_bits)
        assert bits.intersection(register_bit_alloc[part.address]) == set(), row
        register_bit_alloc[part.address] |= bits

        bits = slice_bits(part.argument_bits)
        assert bits.intersection(argument_bit_alloc[name]) == set(), row
        argument_bit_alloc[part.address] |= bits

        if name not in by_name:
            assert part.index == 0, row
            register = Register(name, [part])
            by_name[name] = register
            register_order.append(register)
        else:
            assert len(by_name[name].parts) == part.index
            by_name[name].parts.append(part)

    return addresses, register_order

def format_reg(address):
    return f"reg_{address:04x}"

def render_struct_fields(struct_size, addresses, address_increment, c_type, c_type_size):
    assert address_increment >= c_type_size
    assert address_increment % c_type_size == 0
    next_address = None
    pad_index = 0
    for address, _ in addresses:
        if next_address is None:
            next_address = address
        if address != next_address:
            assert address > next_address, (address, next_address)
            padding = (address - next_address) // c_type_size
            yield f"const {c_type} _pad{pad_index}[{padding}];"
            pad_index += 1

        yield f"{c_type} {format_reg(address)};"
        if c_type_size < address_increment:
            padding = (address_increment - c_type_size) // c_type_size
            yield f"const {c_type} _pad{pad_index}[{padding}];"
            pad_index += 1
        next_address = address + address_increment

    if struct_size is not None:
        assert struct_size % address_increment == 0
        assert struct_size >= next_address
        if struct_size > next_address:
            padding = (struct_size - next_address) // c_type_size
            yield f"const {c_type} _pad{pad_index}[{padding}];"

def render_struct_static_assertions(struct_name, struct_size, addresses, address_increment):
    first_address = addresses[0][0]
    last_address = addresses[-1][0]
    if struct_size is None:
        struct_size = last_address + address_increment
    yield f"static_assert((sizeof ({struct_name})) == {hex(struct_size)} - {hex(first_address)});"
    for address, _ in addresses:
        yield f"static_assert((offsetof ({struct_name}, {format_reg(address)})) == {hex(address)} - {hex(first_address)});"

def mask_from_bits(bit_slice):
    h, l = bit_slice
    mask = 2 ** ((h - l) + 1) - 1
    return mask

def part_get_expression(part):
    _, reg_end = part.register_bits
    _, arg_end = part.argument_bits
    arg_mask = mask_from_bits(part.argument_bits)
    return f"(static_cast<uint32_t>(({format_reg(part.address)} >> {reg_end}) & {hex(arg_mask)}) << {arg_end})"

def part_set_statement(addresses_dict, c_type_real_size, part):
    _, reg_end = part.register_bits
    _, arg_end = part.argument_bits
    arg_mask = mask_from_bits(part.argument_bits)
    assignment = f"{format_reg(part.address)} = (((v >> {arg_end}) & {hex(arg_mask)}) << {reg_end})"
    if len(addresses_dict[part.address]) > 1:
        reg_mask = mask_from_bits(part.register_bits) << reg_end
        inverse_mask = (~reg_mask) & ((2 ** (c_type_real_size * 8)) - 1)
        assignment += f" | ({format_reg(part.address)} & {hex(inverse_mask)});"
    else:
        assignment += ";"
    return assignment

def render_struct_accessors(addresses_dict, c_type, c_type_real_size, registers):
    for register in registers:
        yield f"uint32_t {register.name}() const"
        yield "{"
        get_expression = " | ".join(map(part_get_expression, register.parts))
        yield f"return {get_expression};"
        yield "}"
        yield f"void {register.name}(const uint32_t v)"
        yield "{"
        for part in register.parts:
            yield part_set_statement(addresses_dict, c_type_real_size, part)
        yield "}"
        yield ""

def render_struct(struct_name, struct_size, addresses, address_increment, c_type, c_type_size, c_type_real_size, registers):
    yield f"struct {struct_name} {{"
    yield from render_struct_fields(struct_size, addresses, address_increment, c_type, c_type_size)
    yield ""
    addresses_dict = dict(addresses)
    yield from render_struct_accessors(addresses_dict, c_type, c_type_real_size, registers)
    yield "};"
    yield from render_struct_static_assertions(struct_name, struct_size, addresses, address_increment)

def header():
    yield '#include <stdint.h>'
    yield '#include <stddef.h>'
    yield ""
    yield '#include "type.hpp"'
    yield ""

if __name__ == "__main__":
    input_file = sys.argv[1]
    struct_name = sys.argv[2]
    struct_size = int(sys.argv[3], 16) if len(sys.argv) > 3 else None
    rows = read_input(input_file)
    address_increment = 4
    c_type = "reg32"
    c_type_size = 4
    c_type_real_size = 2
    addresses, registers = group_parts(rows, address_increment)
    render, out = renderer()
    render(header())
    render(render_struct(struct_name, struct_size, addresses, address_increment, c_type, c_type_size, c_type_real_size, registers))
    sys.stdout.write(out.getvalue())
