import sys

start_offset = sys.argv[1]
size = sys.argv[2]
input_file = sys.argv[3]
output_file = sys.argv[4]

assert start_offset.startswith('0x')
start_offset = int(start_offset[2:], 16)
assert size.startswith('0x')
size = int(size[2:], 16)

with open(sys.argv[3], 'rb') as f:
    in_buf = f.read()

with open(sys.argv[4], 'rb') as f:
    out_buf = bytearray(f.read())

for i in range(start_offset, start_offset + size):
    out_buf[i] = in_buf[i]

with open(sys.argv[4], 'wb') as f:
    f.write(out_buf)
