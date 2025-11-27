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

def argb1555(color):
    a = (color >> 15) & 1
    r = (color >> 10) & 31
    g = (color >> 5) & 31
    b = (color >> 0) & 31
    return r << 4, g << 4, b << 4, a * 255

def rgb565(color):
    r = (color >> 11) & 31
    g = (color >> 5) & 63
    b = (color >> 0) & 31
    return r << 3, g << 2, b << 3

def argb4444(color):
    a = (color >> 12) & 15
    r = (color >> 8) & 15
    g = (color >> 4) & 15
    b = (color >> 0) & 15
    return r << 4, g << 4, b << 4, (a << 4) | 0b1111

def get_colors(buf, codebook_ix, decode_pixel):
    codeword = buf[codebook_ix * 2 * 4:][:2 * 4]
    assert len(codeword) == 2 * 4
    colors = struct.unpack('<HHHH', codeword)
    return list(map(decode_pixel, colors))

def log2(n):
    if n == 1:
        return 0
    if n == 2:
        return 1
    if n == 4:
        return 2
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

def decode_vq_indices(data, offset, width, height, decode_pixel, detwiddle=True):
    codebook = pvrt.data[:codebook_size]
    indices = pvrt.data[codebook_size + offset:]
    canvas = [0] * width * height
    for ty in range(height // 2):
        for tx in range(width // 2):
            if detwiddle:
                index_ix = from_xy(tx, ty, width, height)
            else:
                index_ix = ty * (width // 2) + tx
            codebook_ix = indices[index_ix]
            codeword = get_colors(codebook, codebook_ix, decode_pixel)
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

def decode_twiddled(data, offset, width, height, decode_pixel):
    canvas = [0] * width * height
    for y in range(height):
        for x in range(width):
            ix = offset + from_xy(x, y, width, height) * 2
            color, = struct.unpack("<H", data[ix:ix+2])
            canvas[y * width + x] = decode_pixel(color)
    return canvas

def decode_rectangular(data, offset, width, height, decode_pixel):
    canvas = [0] * width * height
    for y in range(height):
        for x in range(width):
            ix = offset + (y * width + x) * 2
            try:
                color, = struct.unpack("<H", data[ix:ix+2])
            except Exception as e:
                print(e)
            canvas[y * width + x] = decode_pixel(color)
    return canvas

in_filename = sys.argv[1]
out_filename = sys.argv[2]

def vq_mip_size(dim):
    n = int(dim // 2)
    if n < 1:
        n = 1
    return n * n

with open(in_filename, 'rb') as f:
    buf = f.read()
pvrt = parse_pvrt_header(buf)
print(pvrt.texture_data_size, hex(pvrt.texture_type), pvrt.width, pvrt.height)
print('texture_type', hex(pvrt.texture_type))

pixel_format = pvrt.texture_type & 0xf
decoders = {
    0: argb1555,
    1: rgb565,
    2: argb4444,
}
if pixel_format not in decoders:
    assert False, ("unsupported pixel format:", pixel_format)
decode_pixel = decoders[pixel_format]

def mm_vq(pvrt, func):
    assert pvrt.width == pvrt.height
    offsets = {
        1: 0x00000,
        2: 0x00001,
        4: 0x00002,
        8: 0x00006,
        16: 0x00016,
        32: 0x00056,
        64: 0x00156,
        128: 0x00556,
        256: 0x01556,
        512: 0x05556,
        1024: 0x15556,
    }
    offset = offsets[pvrt.width]
    print("offset", pvrt.width, hex(offset))
    canvas = func(pvrt.data, offset, pvrt.width, pvrt.width, decode_pixel)
    return canvas

def mm(pvrt, func):
    assert pvrt.width == pvrt.height
    offsets = {
        1: 0x00006,
        2: 0x00008,
        4: 0x00010,
        8: 0x00030,
        16: 0x000B0,
        32: 0x002B0,
        64: 0x00AB0,
        128: 0x02AB0,
        256: 0x0AAB0,
        512: 0x2AAB0,
        1024: 0xAAAB0,
    }
    offset = offsets[pvrt.width]
    print("offset", pvrt.width, hex(offset))
    canvas = func(pvrt.data, offset, pvrt.width, pvrt.width, decode_pixel)
    return canvas

if (pvrt.texture_type & 0xff00) == 0xa00: # rectangular mm
    canvas = mm(pvrt, decode_rectangular)
elif (pvrt.texture_type & 0xff00) == 0x900: # rectangular
    canvas = decode_rectangular(pvrt.data, 0, pvrt.width, pvrt.height, decode_pixel)
elif (pvrt.texture_type & 0xff00) == 0x400: # vq mm
    canvas = mm_vq(pvrt, decode_vq_indices)
elif (pvrt.texture_type & 0xff00) == 0x300: # vq
    canvas = decode_vq_indices(pvrt.data, 0, pvrt.width, pvrt.height, decode_pixel)
elif (pvrt.texture_type & 0xff00) == 0x200: # twiddled mm
    canvas = mm(pvrt, decode_twiddled)
elif (pvrt.texture_type & 0xff00) == 0x100: # twiddled
    canvas = decode_twiddled(pvrt.data, 0, pvrt.width, pvrt.height, decode_pixel)
else:
    assert False, ("unsupported texture type:", hex(pvrt.texture_type))

if pixel_format in {0, 2}: # argb1555, argb4444
    palimage = Image.new('RGBA', (pvrt.width, pvrt.height))
else:
    palimage = Image.new('RGB', (pvrt.width, pvrt.height))
palimage.putdata(canvas)
palimage.save(out_filename)
