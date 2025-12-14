from os import path
import struct
import sys
from itertools import chain
from dataclasses import dataclass
from binascii import unhexlify

storage_size = 128 * 1024
buf = bytearray(storage_size)

storage_blocks_count = storage_size // 512
system_block = 0xff
fat_block = 0xfe
fat_blocks = 1
file_information_block = 0xfd
file_information_blocks = 0xd
block_size = 512

def round_up(n):
    rem = n % block_size
    if rem != 0:
        rem = block_size - rem
    return n + rem

def build_chain(length):
    total_blocks = length // block_size
    print("total blocks", length, total_blocks)
    unused_blocks = storage_blocks_count - total_blocks
    #print(total_blocks, unused_blocks)
    for i in range(total_blocks - 1):
        yield i + 1
    yield 0xfffa

    for i in range(unused_blocks):
        yield 0xfffc

def write_chain(start_entry, chain):
    offset = fat_block * block_size + start_entry * 2
    for entry in chain:
        assert offset < (fat_block + fat_blocks) * block_size
        packed = struct.pack("<H", entry)
        buf[offset+0] = packed[0]
        buf[offset+1] = packed[1]
        offset += 2

def write_image(vms_path):
    with open(vms_path, 'rb') as f:
        vms_buf = f.read()
        assert len(vms_buf) <= (storage_size // 2), len(vms_buf)
    for i, c in enumerate(vms_buf):
        buf[i] = c
    return round_up(len(vms_buf))

def write_file_information_chain():
    block = file_information_block
    for i in range(file_information_blocks - 1):
        chain_ix = file_information_block - (i + 0)
        chain_entry = file_information_block - (i + 1)
        offset = fat_block * block_size + chain_ix * 2
        struct.pack_into("<H", buf, offset, chain_entry)
    struct.pack_into("<H", buf, offset - 2, 0xfffa)

@dataclass
class FileInformation:
    status: int
    copy: int
    start_fat: int
    file_name: bytes
    date: int
    block_size: int
    header: int

file_name_length = 12

def write_file_information(info):
    offset = file_information_block * block_size
    struct.pack_into("<BBH", buf, offset, info.status, info.copy, info.start_fat)
    offset += 4
    assert len(info.file_name) == file_name_length
    for i, c in enumerate(info.file_name):
        buf[offset + i] = c
    offset += file_name_length
    date = b"\x19\x98\x11'\x030\t\x04"
    for i, c in enumerate(date):
        buf[offset + i] = c
    offset += len(date)
    struct.pack_into("<HHI", buf, offset, info.block_size, info.header, 0)

def write_hex(offset, s):
    for i, b in enumerate(unhexlify(s)):
        buf[offset + i] = b

def write_num2(offset, n):
    struct.pack_into("<H", buf, offset, n)

def write_system_area():
    offset = lambda n: system_block * 512 + n

    struct.pack_into("<" + ("B" * 16), buf, offset(0), *([0x55] * 16))

    write_hex(offset(0x10), b"01ffffffff000000000000000000000000000000000000000000000000000000")
    write_hex(offset(0x30), b"1998120604293906")
    write_num2(offset(0x40), 255)
    write_num2(offset(0x44), 255)
    write_num2(offset(0x46), 254)
    write_num2(offset(0x48), 1)
    write_num2(offset(0x4a), 253)
    write_num2(offset(0x4c), 13)
    write_num2(offset(0x50), 200)
    write_num2(offset(0x52), 31)
    write_num2(offset(0x54), 1)

def main():
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} [game.vms] [vmu_image.bin]")
        sys.exit(1)

    vms_path = sys.argv[1]
    image_size = write_image(vms_path)
    image_chain = list(build_chain(image_size))
    assert len(image_chain) == fat_blocks * block_size // 2
    write_chain(0, image_chain)

    file_name = path.split(vms_path.upper())[1][:file_name_length].encode('utf-8')
    append = file_name_length - len(file_name)
    if append > 0:
        file_name += b'\x00' * append

    info = FileInformation(
        status = 0xcc,
        copy = 0x0,
        start_fat = 0x0,
        file_name = file_name,
        date = 0,
        block_size = image_size // block_size,
        header = 1,
    )

    write_file_information_chain()
    write_file_information(info)

    offset = fat_block * 512 + (254 * 2)
    struct.pack_into("<HH", buf, offset, 0xfffa, 0xfffa)

    write_system_area()

    with open(sys.argv[2], 'wb') as f:
        f.write(buf)


main()
