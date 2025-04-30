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
    codebook: list[int]
    indices: list[int]

def parse_pvrt_header(buf):
    header = buf[0:16]
    codebook = buf[16:codebook_size + 16]
    indices = buf[codebook_size + 16:]
    assert len(header) == 16
    assert len(codebook) == codebook_size

    assert header[0:4] == b"PVRT"
    unpacked = struct.unpack('<LLHH', header[4:])
    texture_data_size, texture_type, width, height = unpacked
    print(texture_data_size)
    print(hex(texture_type))
    print(width)
    print(height)
    assert len(indices) + len(codebook) == texture_data_size - 8
    #assert len(indices) == width * height / 4, (len(indices), width * height / 4)
    return PVRT(
        texture_data_size,
        texture_type,
        width,
        height,
        codebook,
        indices,
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

def from_xy(x, y):
    twiddle_ix = 0
    i = 0
    while i <= (20 / 2):
        twiddle_ix |= ((y >> i) & 1) << (i * 2 + 0)
        twiddle_ix |= ((x >> i) & 1) << (i * 2 + 1)
        i += 1
    return twiddle_ix

def decode_vq_indices(codebook, indices, width, height):
    canvas = [0] * width * height
    for ty in range(height // 2):
        for tx in range(width // 2):
            codebook_ix = indices[from_xy(tx, ty)]
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

in_filename = sys.argv[1]
out_filename = sys.argv[2]

with open(in_filename, 'rb') as f:
    buf = f.read()
pvrt = parse_pvrt_header(buf)
canvas = decode_vq_indices(pvrt.codebook, pvrt.indices, pvrt.width, pvrt.height)
#canvas = decode_vq_indices(buf[:256 * 4 * 2], buf[256*4*2:], 256, 256)
print(pvrt.texture_data_size, pvrt.texture_type, pvrt.width, pvrt.height)
palimage = Image.new('RGB', (pvrt.width, pvrt.height))
#palimage = Image.new('RGB', (256, 256))
palimage.putdata(canvas)
palimage.save(out_filename)
