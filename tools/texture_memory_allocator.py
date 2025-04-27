import struct

with open("texture_memory_write_64_read_32.bin", "rb") as f:
    buf = memoryview(f.read())

lookup = [
    struct.unpack("<I", buf[i*4:i*4+4])[0] & 0x7fff_ffff
    for i in range(8 * 1024 * 1024 // 4)
]
#for n in lookup:
#    print(f"{n:08x}")

allocations = {
    "isp_tsp_parameters" : (0x11_c000  , 32 ),
    "object_list"        : (0x09_6000  , 32 ),

    "region_array"       : (0x01_0000  , 0  ),
    "framebuffer0"       : (0x09_6000  , 0  ),

    "framebuffer1"       : (0x09_6000  , 0  ),
    "framebuffer2"       : (0x09_6000  , 0  ),

    "background0"        : (0x00_0040  , 32 ),
    "background1"        : (0x00_0040  , 32 ),
}

def gen_allocations():
    acc = [0x00_0000, 0x40_0000]
    acc_i = 0

    for name, (size, pad) in allocations.items():
        start = acc[acc_i]
        end = start + size - pad
        yield name, (start, end)
        acc[acc_i] += size
        acc_i = int(not acc_i)

def print_alloc():
    for name, (start, end) in gen_allocations():
        print(name, f"{start:06x}", f"{end:06x}")

print_alloc()

def allocate_bitmap(mem, lookup, start, end):
    word_s = start // 4
    end_s = end // 4
    for i in range(word_s, end_s):
        addr64 = lookup[i]
        assert mem[addr64] == 0, (mem[addr64], i, addr64)
        mem[addr64] = (1 << 31) | i

mem64 = [0] * (0x80_0000 // 4)
for name, (start, end) in gen_allocations():
    allocate_bitmap(mem64, lookup, start, end)

from PIL import Image
im = Image.new("RGB", (1024, 2048))
width, height = im.size
for y in range(height):
    for x in range(width):
        color = (255, 128, 0) if mem64[y * 1024 + x] != 0 else (0, 255, 0)
        im.putpixel((x, y), color)

im.save("map.png")
texture_address = None
for i in reversed(range(len(mem64))):
    if mem64[i] != 0:
        texture_address = (i + 1) * 4
        break
print(f"texture {texture_address:06x}")
