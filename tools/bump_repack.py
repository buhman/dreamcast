from PIL import Image
import sys
import struct

im = Image.open(sys.argv[1])
im = im.convert("RGB")
width, height = im.size

with open(sys.argv[2], 'wb') as f:
    for v in range(height):
        for u in range(width):
            px = im.getpixel((u, v))
            r, s, _ = px
            value = (s << 8) | (r << 0)
            f.write(struct.pack("<H", value))
