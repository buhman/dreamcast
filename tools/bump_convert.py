from PIL import Image
import sys
import math
from decimal import Decimal
from math import acos, asin, pi, isnan
import struct

im = Image.open(sys.argv[1])
im = im.convert("RGB")

out = Image.new("RGB", im.size)

width, height = im.size

def mul(a, b):
    return (a[0] * b[0], a[1] * b[1], a[2] * b[2])

def dot(a, b):
    x, y, z = mul(a, b)
    return x + y + z

def magnitude(v):
  return math.sqrt(dot(v, v));

def remap(value, l1, h1, l2, h2):
    if (value == 255):
        value = 256
    d = Decimal
    mul = (d(h2) - d(l2)) / (d(h1) - d(l1))
    output = d(l2) + (d(value) - d(l1)) * mul

    if output == 0:
        output = 0.0000000001
    return float(output)

def remap_normal(x, y, z):
    vec = (
        remap(x, 0, 256, -1, 1),
        remap(y, 0, 256, -1, 1),
        remap(z, 0, 256, 1, -1),
    )
    div = 1.0 / magnitude(vec)
    return vec

tests = [
    ((0,0,-1), (128,128,255)),
    ((1,1,0), (255,255,128)),
    ((1,0,0), (255,128,128)),
    ((0,1,0), (128,255,128)),
    ((-1,0,0), (0,128,128)),
    ((0,-1,0), (128,0,128)),
    ((-1,-1,0), (0,0,128)),
]
for xyz, rgb in tests:
    rx, ry, rz = remap_normal(*rgb)
    if abs(rx - 0) < 0.01:
        rx = 0
    if abs(ry - 0) < 0.01:
        ry = 0
    if abs(rz - 0) < 0.01:
        rz = 0
    r = (rx, ry, rz)
    #print(r, xyz)
    #assert r == xyz, (r, xyz)

x_vec = (1, 0, 0)

buf_out = [0] * width * height

for v in range(height):
    for u in range(width):
        px = im.getpixel((u, v))
        x, y, z = remap_normal(*px)
        #assert z >= 0, (x, y, z)

        pos_xy = (x, y, 0)

        r = 0
        if pos_xy[0] != 0 and pos_xy[1] != 0:
            mag = (magnitude(x_vec) * magnitude(pos_xy))
            assert mag != 0, mag
            d = dot(x_vec, pos_xy)
            if d == 0:
                print(pos_xy)
            r = acos(d / mag)
            if pos_xy[1] < 0:
                r = 2 * pi - r
            assert not isnan(r)
        zz = (-z) if z < 0 else 0
        s = asin(zz)
        #s = zz * pi / 2

        assert r >= 0 and r <= pi * 2, r
        assert s >= 0 and s <= pi / 2, (z, zz, s)

        if u >= 163 and u <= 165:
            if v >= 334 and v <= 336:
                print((u, v), (px[2]), z, s)

        r_255 = int(remap(r, 0, pi * 2, 0, 255))
        s_255 = int(remap(s, 0, pi / 2, 0, 255))

        assert r_255 >= 0 and r_255 <= 255, r_255
        assert s_255 >= 0 and s_255 <= 255, s_255

        buf_out[(v * width) + u] = (s_255 << 8) | (r_255 << 0)

        #int((px[0] * px[1]) / 255)
        out.putpixel((u, v), (0, s_255, 0))

out.save("test.png")

with open(sys.argv[2], 'wb') as f:
    for v in buf_out:
        f.write(struct.pack("<H", v))
