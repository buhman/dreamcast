# 565

from PIL import Image
import struct

with open('framebuffer.bin', 'rb') as f:
    buf = f.read()

image = Image.new('RGB', (640, 480), color=None)

for i in range(len(buf) // 2):
    b, = struct.unpack('<H', buf[i*2:(i+1)*2])
    blue  = (b >> 0 ) & 31
    green = (b >> 5 ) & 63
    red   = (b >> 11) & 31

    x = i % 640
    y = i // 640
    image.putpixel((x, y), (red * 8, green * 4, blue * 8))

image.save("framebuffer.png")
