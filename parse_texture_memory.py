from dataclasses import dataclass
import sys
import struct
from pprint import pprint, pformat
import textwrap

with open(sys.argv[1], "rb") as f:
    mem = memoryview(f.read())

@dataclass
class buf_parser:
    mem: memoryview
    address: int

def u32(buf):
    value, = struct.unpack("<I", buf.mem[buf.address:buf.address+4])
    buf.address += 4
    return value

def f32(buf):
    value, = struct.unpack("<f", buf.mem[buf.address:buf.address+4])
    buf.address += 4
    return value

@dataclass
class tile:
    flags: set[str]
    x: int
    y: int

def region_array_tile(value):
    last_region = (value >> 31) & 1
    z_clear = (value >> 30) & 1
    pre_sort = (value >> 29) & 1
    flush_accumulate = (value >> 28) & 1
    tile_y_position = (value >> 8) & 0x3f
    tile_x_position = (value >> 2) & 0x3f

    flags = set()
    if last_region:
        flags.add("last_region")
    if z_clear:
        flags.add("z_clear")
    if pre_sort:
        flags.add("pre_sort")
    if flush_accumulate:
        flags.add("flush_accumulate")
    return tile(flags, tile_x_position, tile_y_position)

@dataclass
class region_array_entry:
    tile: int
    opaque_list_pointer: int
    modifier_volume_list_pointer: int
    translucent_list_pointer: int
    translucent_modifier_volume_list_pointer: int
    punch_through_list_pointer: int

    def __init__(self):
        pass

def parse_region_array_entry(buf):
    entry = region_array_entry()
    entry.tile = region_array_tile(u32(buf))
    entry.opaque_list_pointer = u32(buf)
    entry.modifier_volume_list_pointer = u32(buf)
    entry.translucent_list_pointer = u32(buf)
    entry.translucent_modifier_volume_list_pointer = u32(buf)
    entry.punch_through_list_pointer = u32(buf)
    return entry

def parse_region_array(mem, address):
    buf = buf_parser(mem, address)
    while True:
        entry = parse_region_array_entry(buf)
        yield entry
        if "last_region" in entry.tile.flags:
            break

@dataclass
class object_list_array:
    number: int
    shadow: int
    skip: int
    start: int

def print_array(value):
    number = (value >> 25) & 0b1111
    shadow = (value >> 24) & 1
    skip = (value >> 21) & 0b111
    start = value & ((1 << 21) - 1)
    print(f'    number: {number}')
    print(f'    shadow: {shadow}')
    print(f'    skip: {skip}')
    print(f'    start: {start}')
    return object_list_array(number, shadow, skip, start)

@dataclass
class isp_tsp_instruction_word:
    depth_compare_mode: str
    culling_mode: str
    z_write_disable: int
    texture: int
    offset: int
    gouraud_shading: int
    _16bit_uv: int
    cache_bypass: int
    dcalc_ctrl: int

def parse_isp_tsp_instruction_word(value):
    def depth_compare_mode(value):
        value = (value >> 29) & 0b111
        if value == 0: return "never"
        if value == 1: return "less"
        if value == 2: return "equal"
        if value == 3: return "less_or_equal"
        if value == 4: return "greater"
        if value == 5: return "not_equal"
        if value == 6: return "greater_or_equal"
        if value == 7: return "always"

    def culling_mode(value):
        value = (value >> 27) & 0b11
        if value == 0: return "no_culling"
        if value == 1: return "cull_if_small"
        if value == 2: return "cull_if_negative"
        if value == 3: return "cull_if_positive"

    z_write_disable = (value >> 26) & 1
    texture = (value >> 25) & 1
    offset = (value >> 24) & 1
    gouraud_shading = (value >> 23) & 1
    _16bit_uv = (value >> 22) & 1
    cache_bypass = (value >> 21) & 1
    dcalc_ctrl = (value >> 20) & 1
    print(hex(value))
    return isp_tsp_instruction_word(
        depth_compare_mode(value),
        culling_mode(value),
        z_write_disable,
        texture,
        offset,
        gouraud_shading,
        _16bit_uv,
        cache_bypass,
        dcalc_ctrl,
    )

def print_params(mem, ol_array, num_vertices):
    # skip

    # This field specifies the data size (* 32 bits) for one vertex in the
    # ISP/TSP Parameters.  Normally, the actual data size is "skip + 3,"

    buf = buf_parser(mem, ol_array.start)
    isp_tsp = u32(buf)
    tsp = u32(buf)
    texture = u32(buf)
    print(textwrap.indent(pformat(parse_isp_tsp_instruction_word(isp_tsp)), '    '))

    vertex_size = (ol_array.skip + 3) * 4
    while num_vertices > 0:
        start = buf.address
        x = f32(buf)
        y = f32(buf)
        z = f32(buf)
        base_color = u32(buf)
        print(f"{x:7.3f} {y:7.3f} {z:7.3f} {base_color:x}")
        buf.address = start + vertex_size
        num_vertices -= 1

def parse_object_list_data(mem, value):
    if ((value >> 31) & 1) == 0:
        print('  triangle:', hex(value))
    elif ((value >> 29) & 0b111) == 0b100:
        print('  triangle array:', hex(value))
        ol_array = print_array(value)
        print_params(mem, ol_array, 3)
    elif ((value >> 29) & 0b111) == 0b101:
        print('  quad array:', hex(value))
        ol_array = print_array(value)
        print_params(mem, ol_array, 4)
    elif ((value >> 29) & 0b111) == 0b111:
        print('  block link:', hex(value))
        end_of_list = (value >> 28) & 1
        if end_of_list:
            print('    [end of list]')
        return end_of_list
    else:
        assert False, value

def parse_object_list(mem, address):
    if (address & (1 << 31)) != 0:
        return
    buf = buf_parser(mem, address)
    while True:
        end_of_list = parse_object_list_data(mem, u32(buf))
        if end_of_list:
            break

def parse_object_lists(mem, region_array_entry):
    print(region_array_entry.tile)
    parse_object_list(mem, region_array_entry.opaque_list_pointer)
    parse_object_list(mem, region_array_entry.modifier_volume_list_pointer)
    parse_object_list(mem, region_array_entry.translucent_list_pointer)
    parse_object_list(mem, region_array_entry.translucent_modifier_volume_list_pointer)
    parse_object_list(mem, region_array_entry.punch_through_list_pointer)


region_array = 0x296000
framebuffer = 0x200000
region_array_entries = list(parse_region_array(mem, region_array))

for region_array_entry in region_array_entries:
    parse_object_lists(mem, region_array_entry)

#with open('framebuffer.data', 'wb') as f:
#    f.write(mem[framebuffer:framebuffer + 640 * 480 * 2])
