from collections import defaultdict
import sys
from dataclasses import dataclass
from generate import renderer

from csv_input import read_input_headerless

def one_bit(register, bit, value):
    v = int(value, 16)
    if v == 0:
        return None
    elif v == 1:
        return f"{register}::{bit}"
    else:
        assert False, (register, bit, value)

def enum(enums):
    def do(register, bit, value):
        v = int(value, 16)
        return f"{register}::{bit}::{enums[v]}"
    return do

special = {
    ("spg_control", "pal"): one_bit,
    ("spg_control", "ntsc"): one_bit,
    ("spg_control", "interlace"): one_bit,
    ("spg_control", "sync_direction"): enum({0: "input", 1: "output"}),
    ("vo_control", "pixel_double"): one_bit,
    ("fb_r_ctrl", "vclk_div"): enum({0: "pclk_vclk_2", 1: "pclk_vclk_1"}),
}

def parse_regs(rows):
    last_register = None
    for row in rows:
        register, bit, *values0 = row
        if register == "":
            assert last_register is not None
            register = last_register
        last_register = register
        values = [v for v in values0]
        yield register, bit, values

@dataclass
class Bit:
    name: str
    value: int

@dataclass
class Register:
    name: str
    bits: list[Bit]

@dataclass
class Mode:
    name: str
    registers: list[Register]

def group_by_register(regs):
    by_register = defaultdict(list)
    register_names = []
    for register, bit, value in regs:
        if register not in register_names:
            register_names.append(register)
        by_register[register].append(Bit(
            name=bit.lower(),
            value=value
        ))

    return [
        Register(
            name=register.lower(),
            bits=by_register[register]
        )
        for register in register_names
    ]

def transpose_by_name(format_names, regs):
    by_name = defaultdict(list)
    for register, bit, values in regs:
        assert len(values) == len(format_names)
        for name, value in zip(format_names, values):
            by_name[name].append((register, bit, value))
    return [
        Mode(
            name=name,
            registers=group_by_register(by_name[name])
        )
        for name in format_names
    ]

def render_bit(register, bit, value):
    if (register, bit) in special:
        return special[(register, bit)](register, bit, value)
    else:
        return f"{register}::{bit}(0x{value})"

def render_mode(mode, max_len):
    for register in mode.registers:
        i_out = 0
        for bit in register.bits:
            lhs = f".{register.name}" if i_out == 0 else ""
            sym = '=' if i_out == 0 else "|"
            _render_bit = render_bit(register.name, bit.name, bit.value)
            if _render_bit is not None:
                i_out += 1
                yield f"{lhs.ljust(max_len)} {sym} {_render_bit}"

        yield f"{''.ljust(max_len)} ,"


def render_modes(modes, max_len):
    for mode in modes:
        yield f"const struct mode {mode.name.lower()} = {{"
        yield from render_mode(mode, max_len)
        yield "};"

def render_namespace(modes, max_len):
    yield "namespace video_output {"
    yield from render_modes(modes, max_len)
    yield "}"

def render_header():
    yield "#include <cstdint>"
    yield ""
    yield '#include "core_bits.hpp"'
    yield '#include "video_output.hpp"'
    yield ""

def max_length(regs):
    max_length = 0
    for register, *_ in regs:
        if len(register) > max_length:
            max_length = len(register)
    return max_length + 1

if __name__ == "__main__":
    rows = read_input_headerless(sys.argv[1])
    formats0, *regs0 = rows
    b0, b1, *format_names = formats0
    assert (b0, b1) == ("", ""), (b0, b1)
    regs = list(parse_regs(regs0))
    modes = transpose_by_name(format_names, regs)
    max_length = max_length(regs)
    render, out = renderer()
    render(render_header())
    render(render_namespace(modes, max_length))
    sys.stdout.write(out.getvalue())
