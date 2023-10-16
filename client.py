import serial
import struct
import sys
import time

dest = 0xac02_0000

ret = []

def sync(ser, b):
    l = []
    for i, c in enumerate(b):
        if i % 32 == 0:
            print(i, end=' ')
            sys.stdout.flush()
        ser.write(bytes([c]))
        ser.flush()
        ser.flushInput()
        ser.flushOutput()
        time.sleep(0.01)
        while ser.in_waiting > 0:
            res = ser.read(ser.in_waiting)
            for c in res:
                l.append(c)
            time.sleep(0.01)

    time.sleep(1)
    res = ser.read(ser.in_waiting)
    for c in res:
        l.append(c)

    return bytes(l)

def do(ser, b):
    ser.flush()
    ser.flushInput()
    ser.flushOutput()

    ret = sync(ser, b'DATA')
    print(ret)
    size = len(b)
    args = struct.pack("<II", size, dest)
    print("dargs", args)
    ret = sync(ser, args)
    print(ret)

    ret = sync(ser, b)
    print(ret[-5:])
    if ret[:-5] != b:
        print("ret != b; dumped to asdf.bin")
        with open('asdf.bin', 'wb') as f:
            f.write(ret[:-5])

    ret = sync(ser, b'JUMP')
    args = struct.pack("<I", dest)
    ser.write(args)
    print()
    while True:
        b = ser.read(1)
        if b:
            sys.stderr.buffer.write(b)
            sys.stderr.flush()

with open(sys.argv[1], 'rb') as f:
    b = f.read()

with serial.Serial('/dev/ttyUSB0', 120192, timeout=1) as ser:
    do(ser, b)
