from PIL import Image
import sys
from glob import glob
from generate import renderer
from dataclasses import dataclass
from os import path

def read_texture_names():
    buf = sys.stdin.read()
    lines = buf.strip().split('\n')
    return lines

def glob_and_filter(name):
    filenames = glob(f"*/{name}.tga") + glob(f"*/{name}.jpg")
    assert len(filenames) in {0, 1}, filenames
    filename = filenames[0] if filenames else None
    return filename

def image_size(filename):
    if filename is None:
        return (0, 0)
    with Image.open(filename) as im:
        return im.size

def npot(v):
    v -= 1
    v |= v >> 1
    v |= v >> 2
    v |= v >> 4
    v |= v >> 8
    v |= v >> 16
    v += 1
    return v

@dataclass
class Size:
    w: int
    h: int

@dataclass
class Texture:
    name: str
    filename: str
    real_size: Size
    npot_size: Size
    offset: int

mipmapped = False

def mip_size(n):
    if n == 0:
        return 0
    size = 6
    while n > 0:
        size += n * n * 2
        n >>= 1
    return size

assert mip_size(256) == 0x2aab0, hex(mip_size(256))

def texture_metadata():
    global mipmapped

    names = read_texture_names()
    acc = 0
    for name in names:
        filename = glob_and_filter(name)
        w, h = image_size(filename)
        nw, nh = npot(w), npot(h)

        if w > 256:
            name = None
            filename = None
            w, h, nw, nh = 0, 0, 0, 0
        elif filename:
            print(filename)
        yield Texture(
            name,
            filename,
            Size(w, h),
            Size(nw, nh),
            acc
        )

        if mipmapped:
            assert w == h and nw == w, (w, h)
            acc += mip_size(w)
        else:
            acc += nw * h * 2
    assert acc <= (0x80_0000 - 0x37_1800), acc

def name_to_bin(filename):
    if filename is None:
        return None
    else:
        name, ext = path.splitext(filename)
        return f"_binary_bsp_" + name.replace('/', '_').replace('.', '_').replace('-', '_') + "_data"

def uv_mul(texture):
    u = 0 if texture.npot_size.w == 0 else texture.real_size.w / texture.npot_size.w
    v = 0 if texture.npot_size.h == 0 else texture.real_size.h / texture.npot_size.h
    return u, v

def render_texture_metadata(texture):
    name = name_to_bin(texture.filename)
    u, v = uv_mul(texture)
    assert u == 1.0 or u == 0.0
    start = "0" if name is None else f"&{name}_start"
    size = "0" if name is None else f"&{name}_size"
    yield "{"
    yield f".start = (void *){start},"
    yield f".size = (uint32_t){size},"
    yield f".offset = {texture.offset},"
    yield f".width = {texture.npot_size.w},"
    yield f".height = {texture.npot_size.h},"
    #yield f".u_mul = {u}, // {texture.real_size.w}"
    yield f".v_mul = {v}, // {texture.real_size.h}"
    yield "},"

def render_texture_metadatas():
    for texture in texture_metadata():
        yield from render_texture_metadata(texture)

def main():
    global mipmapped
    out_filename = sys.argv[1]
    is_mipmapped = sys.argv[2]
    assert is_mipmapped in {"mipmapped", "non_mipmapped"}
    mipmapped = is_mipmapped == "mipmapped"

    render, out = renderer()
    render(render_texture_metadatas())
    with open(out_filename, "w") as f:
        f.write(out.getvalue())

main()
