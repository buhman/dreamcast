import sys

width = 640
height = 480

with open(sys.argv[1], 'rb') as f:
    b = f.read()
    assert len(b) == width * height, len(b)

buf = []

for i in range(len(b) // 8):
    ix = i * 8
    px = b[ix:ix + 8]
    assert all(p in {0, 1} for p in px)
    out = (
          (px[0] << 7)
        | (px[1] << 6)
        | (px[2] << 5)
        | (px[3] << 4)
        | (px[4] << 3)
        | (px[5] << 2)
        | (px[6] << 1)
        | (px[7] << 0)
    )
    buf.append(out)

assert len(buf) == width * height // 8

with open(sys.argv[2], 'wb') as f:
    f.write(bytes(buf))
