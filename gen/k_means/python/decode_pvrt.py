from dataclasses import dataclass
import sys
import struct
from PIL import Image

codebook_size = 256 * 2 * 4

@dataclass
class PVRT:
    texture_data_size: int
    texture_type: int
    width: int
    height: int
    data: list[int]

def parse_pvrt_header(buf):
    header = buf[0:16]
    #codebook = buf[16:codebook_size + 16]
    #indices = buf[codebook_size + 16:]
    data = buf[16:]
    assert len(header) == 16
    assert header[0:4] == b"PVRT"
    unpacked = struct.unpack('<LLHH', header[4:])
    texture_data_size, texture_type, width, height = unpacked
    #print(texture_data_size)
    #print("texture type", hex(texture_type))
    #print(width)
    #print(height)
    #assert len(indices) + len(codebook) == texture_data_size - 8, (len(indices) + len(codebook), texture_data_size - 8)
    #assert len(indices) == width * height / 4, (len(indices), width * height / 4)
    return PVRT(
        texture_data_size,
        texture_type,
        width,
        height,
        data,
    )

def rgb24(color):
    r = (color >> 11) & 31
    g = (color >> 5) & 63
    b = (color >> 0) & 31
    return r << 3, g << 2, b << 3

def get_colors(buf, codebook_ix):
    codeword = buf[codebook_ix * 2 * 4:][:2 * 4]
    assert len(codeword) == 2 * 4
    colors = struct.unpack('<HHHH', codeword)
    return list(map(rgb24, colors))

def log2(n):
    if n == 8:
        return 3
    if n == 16:
        return 4
    if n == 32:
        return 5
    if n == 64:
        return 6
    if n == 128:
        return 7
    if n == 256:
        return 8
    if n == 512:
        return 9
    if n == 1024:
        return 10
    assert False, n

def from_xy(x, y, width, height):
    # maximum texture size       : 1024x1024
    # maximum 1-dimensional index: 0xfffff
    # bits                       : 19-0

    # y bits: 0, 2, 4, 6, 8, 10, 12, 14, 16, 18
    # x bits: 1, 3, 5, 7, 9, 11, 13, 15, 17, 19

    width_max = log2(width);
    height_max = log2(height);

    twiddle_ix = 0
    i = 0
    while i < (20 / 2):
        if i < width_max and i < height_max:
            twiddle_ix |= ((y >> i) & 1) << (i * 2 + 0)
            twiddle_ix |= ((x >> i) & 1) << (i * 2 + 1)
        elif i < width_max:
            twiddle_ix |= ((x >> i) & 1) << (i + height_max)
        elif i < height_max:
            twiddle_ix |= ((y >> i) & 1) << (i + width_max)
        else:
            break
        i += 1

    return twiddle_ix

def decode_vq_indices(data, width, height):
    codebook = data[:codebook_size]
    indices = data[codebook_size:]
    canvas = [0] * width * height
    for ty in range(height // 2):
        for tx in range(width // 2):
            codebook_ix = indices[from_xy(tx, ty, width, height)]
            codeword = get_colors(codebook, codebook_ix)
            ai = ((ty * 2) + 0) * width + ((tx * 2) + 0)
            bi = ((ty * 2) + 1) * width + ((tx * 2) + 0)
            ci = ((ty * 2) + 0) * width + ((tx * 2) + 1)
            di = ((ty * 2) + 1) * width + ((tx * 2) + 1)
            #print(width, height, ai, ty, tx)
            canvas[ai] = codeword[0]
            canvas[bi] = codeword[1]
            canvas[ci] = codeword[2]
            canvas[di] = codeword[3]
    return canvas

def decode_twiddled(data, width, height):
    canvas = [0] * width * height
    for y in range(height):
        for x in range(width):
            ix = from_xy(x, y, width, height) * 2
            color, = struct.unpack("<H", data[ix:ix+2])
            canvas[y * width + x] = rgb24(color)
    return canvas

in_filename = sys.argv[1]
out_filename = sys.argv[2]

with open(in_filename, 'rb') as f:
    buf = f.read()
pvrt = parse_pvrt_header(buf)
print(pvrt.texture_data_size, hex(pvrt.texture_type), pvrt.width, pvrt.height)
if (pvrt.texture_type & 0xff00) == 0x300: # vq
    canvas = decode_vq_indices(pvrt.data, pvrt.width, pvrt.height)
elif (pvrt.texture_type & 0xff00) == 0x100: # twiddled
    canvas = decode_twiddled(pvrt.data, pvrt.width, pvrt.height)
else:
    assert False, ("unsupported texture type:", hex(pvrt.texture_type))

palimage = Image.new('RGB', (pvrt.width, pvrt.height))
palimage.putdata(canvas)
palimage.save(out_filename)
