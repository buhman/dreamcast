import serial
import struct
import sys
import time

#dest = 0xac21_0000
dest = 0xac02_0000

ret = []

def sync(ser, b, wait=0.1):
    l = []
    for i, c in enumerate(b):
        if i % 32 == 0 and i != 0:
            print(i, end=' ')
            sys.stdout.flush()
        ser.write(bytes([c]))
        time.sleep(1000 / 1000000)
        if ser.in_waiting == 0:
            time.sleep(0.001)
        while ser.in_waiting > 0:
            res = ser.read(ser.in_waiting)
            l.extend(res)
            time.sleep(0.001)

    time.sleep(wait)
    res = ser.read(ser.in_waiting)
    l.extend(res)

    return bytes(l)

def symmetric(ser, b):
    l = []
    mem = memoryview(b)
    i = 0
    chunk_size = 384

    while i < len(b):
        if i % 1024 == 0:
            print(i, end=' ')
            sys.stdout.flush()
        while True:
            if ser.in_waiting:
                res = ser.read(ser.in_waiting)
                l.extend(res)
            if len(l) + chunk_size >= i:
                break

        chunk_size = min(chunk_size, len(b) - i)
        assert chunk_size > 0, chunk_size
        ser.write(mem[i:i+chunk_size])
        i += chunk_size

    orig_count = 1000
    count = orig_count
    while count > 0:
        if len(l) >= len(b):
            break
        time.sleep(1 / orig_count)
        if ser.in_waiting:
            res = ser.read(ser.in_waiting)
            l.extend(res)
        count -= 1

    return bytes(l)

def start_data(ser, b):
    _ = ser.read(ser.in_waiting)

    size = len(b)
    args = struct.pack("<II", size, dest)
    ret = sync(ser, b'DATA' + args)
    if ret != b'data\n':
        print(".", end=' ')
        sys.stdout.flush()
        sync(ser, b'prime', wait=0)
        start_data(ser, b)
        return
    print("\nDATA")

def do(ser, b):
    start_data(ser, b)

    start = time.monotonic()
    ret = symmetric(ser, b)
    #print("\nHERE", len(ret), len(b))
    end = time.monotonic()
    duration = end - start
    print("\n\nduration:", duration, "\n\n")
    print(ret[-5:])
    if ret[:-5] != b:
        print("ret != b; dumped to asdf.bin")
        with open('asdf.bin', 'wb') as f:
            f.write(ret[:-5])
        print("did not jump")
        return

    args = struct.pack("<I", dest)
    ret = sync(ser, b'JUMP' + args, wait=0)
    print()
    console(ser)

def console(ser):
    while True:
        b = ser.read(1)
        if b:
            sys.stderr.buffer.write(b)
            sys.stderr.flush()


with open(sys.argv[1], 'rb') as f:
    b = f.read()

def baudrate_from_scbrr2(n):
    return 1562500 / (n+1)

def change_rate(old_rate, new_rate):
    with serial.Serial(port='/dev/ttyUSB0',
                       baudrate=baudrate_from_scbrr2(old_rate),
                       bytesize=serial.EIGHTBITS,
                       parity=serial.PARITY_NONE,
                       stopbits=serial.STOPBITS_ONE,
                       timeout=1,
                       xonxoff=False,
                       #rtscts=False,
                       rtscts=True,
                       ) as ser:
        buf = b'\x00\x00\x00\x00'
        start_data(ser, buf)
        ret = symmetric(ser, buf)
        print('ret', ret)

        print("change rate", int(baudrate_from_scbrr2(new_rate)))
        args = struct.pack("<I", new_rate & 0xff)
        ret = sync(ser, b'RATE' + args, wait=1)
        print(ret)

old_rate = 1
new_rate = 1
if old_rate != new_rate:
    change_rate(old_rate, new_rate)

with serial.Serial(port='/dev/ttyUSB0',
                   baudrate=baudrate_from_scbrr2(new_rate),
                   bytesize=serial.EIGHTBITS,
                   parity=serial.PARITY_NONE,
                   stopbits=serial.STOPBITS_ONE,
                   timeout=1,
                   xonxoff=False,
                   #rtscts=False,
                   rtscts=True,
                   ) as ser:
    #console(ser)
    print("waiting: ", end=' ')
    sys.stdout.flush()
    do(ser, b)
