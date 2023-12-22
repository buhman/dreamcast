import sys

def from_xy(x: int, y: int) -> int:
    # maximum texture size       : 1024x1024
    # maximum 1-dimensional index: 0xfffff
    # bits                       : 19-0

    twiddle_ix = 0
    for i in range(0, (19 // 2) + 1):
        twiddle_ix |= ((y >> i) & 1) << (i * 2 + 0)
        twiddle_ix |= ((x >> i) & 1) << (i * 2 + 1)

    return twiddle_ix

assert from_xy(0b000, 0b000) == 0
assert from_xy(0b001, 0b000) == 2
assert from_xy(0b010, 0b000) == 8
assert from_xy(0b011, 0b000) == 10
assert from_xy(0b100, 0b000) == 32
assert from_xy(0b101, 0b000) == 34
assert from_xy(0b110, 0b000) == 40
assert from_xy(0b111, 0b000) == 42

assert from_xy(0b000, 0b001) == 1
assert from_xy(0b000, 0b010) == 4
assert from_xy(0b000, 0b011) == 5
assert from_xy(0b000, 0b100) == 16
assert from_xy(0b000, 0b101) == 17
assert from_xy(0b000, 0b110) == 20
assert from_xy(0b000, 0b111) == 21

def from_ix(z_curve_ix: int) -> tuple[int, int]:
    x_y = [0, 0]
    xyi = 0
    while z_curve_ix != 0:
        x_y[(xyi + 1) % 2] |= (z_curve_ix & 1) << (xyi // 2)
        z_curve_ix >>= 1
        xyi += 1
    return tuple(x_y)

assert from_ix(17) == (0b000, 0b101)
assert from_ix(21) == (0b000, 0b111)
assert from_ix(42) == (0b111, 0b000)

"""
def texture(src: list[int],
            width: int, height: int) -> list[int]:
    dst = [0] * (width * height)

    for y in range(0, height):
        for x in range(0, width):
            twiddle_ix = from_xy(x, y)
            value = src[y * width + x]
            dst[twiddle_ix] = value

    return dst
"""

import random
from sizes import sizes as _sizes
from colorsys import hsv_to_rgb

def all_colors(num):
    for i in range(num):
        hue = i / (num - 1)
        rgb = hsv_to_rgb(hue, 1.0, 1.0)
        def color():
            for i in rgb:
                yield int(i * 255)
        yield tuple(color())

def random_colors(num):
    l = list(all_colors(num))
    random.shuffle(l)
    return l

def area_pixels(x_off, y_off, width, height):
    for x in range(height):
        for y in range(width):
            px_ix = (x_off + x, y_off + y)
            yield px_ix

max_size = (1, 1)

def pack_into(texture: dict[tuple[int, int], int],
              width_height: tuple[int, int],
              z_curve_ix: int):
    global max_size
    # ignore passed z_curve_ix
    z_curve_ix = 0

    width, height = width_height
    if width == 0 or height == 0:
        return (0, 0), z_curve_ix

    while True:
        x_off, y_off = from_ix(z_curve_ix)
        if x_off >= max_size[0] and y_off >= max_size[0]:
            if max_size[0] == max_size[1]:
                max_size = (max_size[0], max_size[1] * 2)
            else:
                max_size = (max_size[0] * 2, max_size[1])
            z_curve_ix = 0

        if all((pixel not in texture) and (pixel[0] < max_size[0] and pixel[1] < max_size[1])
               for pixel in area_pixels(x_off, y_off, width, height)):
            #x, y = x_off + width - 1, y_off + height - 1
            return (x_off, y_off), z_curve_ix
        else:
            z_curve_ix += 1

def sort_by_area(width_height):
    width, height = width_height
    area = width * height
    return area

max_ix = 0

def insert_into_texture(texture, color_ix, x_off__y_off, width_height):
    global max_ix
    x_off, y_off = x_off__y_off
    width, height = width_height
    for px_ix in area_pixels(x_off, y_off, width, height):
        assert px_ix not in texture, px_ix
        ix = from_xy(*px_ix)
        if ix > max_ix:
            max_ix = ix
        texture[px_ix] = color_ix

def pack_all(sizes):
    global max_size
    max_size = (1, 1)
    global max_ix
    max_ix = 0

    sorted_sizes = sorted(sizes, key=sort_by_area, reverse=True)
    z_curve_ix = 0
    texture = dict()
    for color_ix, width_height in enumerate(sorted_sizes):
        x_off__y_off, z_curve_ix = pack_into(texture, width_height, z_curve_ix)
        insert_into_texture(texture, color_ix, x_off__y_off, width_height)
        #if color_ix == 2:
        #    break
    return texture

def ppm(texture: dict, num_colors):
    colors = random_colors(num_colors)
    max_x = max(px[0] for px in texture.keys())
    max_y = max(px[1] for px in texture.keys())
    width = max_x + 1
    height = max_y + 1
    print("      max xy:", max_x, max_y, file=sys.stderr)
    print("max curve ix:", from_xy(max_x, max_y), file=sys.stderr)

    yield "P3"
    yield f"{width} {height}"
    yield "255"
    for y in range(0, height):
        for x in range(0, width):
            if (x, y) in texture:
                i = texture[(x, y)]
                color = colors[i]
                yield " ".join(map(str, color))
            else:
                yield "0 0 0"

if __name__ == '__main__':
    texture = pack_all((x, y) for (y, x) in  _sizes)
    print("ideal size", sum(x * y for x, y in _sizes), file=sys.stderr)
    image = list(ppm(texture, len(_sizes)))
    print("\n".join(image))
    pass
