import serial
import struct
import sys
import time

#dest = 0xac21_0000
dest = 0xac02_0000

ret = []

def sync(ser, b, wait=1):
    l = []
    for i, c in enumerate(b):
        if i % 32 == 0:
            print(i, end=' ')
            sys.stdout.flush()
        ser.write(bytes([c]))
        time.sleep(1000 / 1000000)
        if ser.in_waiting == 0:
            time.sleep(0.01)
        while ser.in_waiting > 0:
            res = ser.read(ser.in_waiting)
            l.extend(res)
            time.sleep(0.01)

    time.sleep(wait)
    res = ser.read(ser.in_waiting)
    l.extend(res)

    return bytes(l)

def symmetric(ser, b):
    l = []
    for i, c in enumerate(b):
        if i % 32 == 0:
            print(i, end=' ')
            sys.stdout.flush()
        while True:
            if ser.in_waiting:
                res = ser.read(ser.in_waiting)
                l.extend(res)
            if len(l) + 8 >= i:
                break
            else:
                time.sleep(0.001)

        ser.write(bytes([c]))

    time.sleep(0.1)
    res = ser.read(ser.in_waiting)
    l.extend(res)

    return bytes(l)

def do(ser, b):
    _ = ser.read(ser.in_waiting)
    ser.flush()
    ser.flushInput()
    ser.flushOutput()
    _ = ser.read(ser.in_waiting)

    ret = sync(ser, b'DATA')
    print(ret)
    size = len(b)
    args = struct.pack("<II", size, dest)
    print("dargs", args)
    ret = sync(ser, args)
    print(ret)
    if ret != b'data\n':
        do(ser, b)

    ret = symmetric(ser, b)
    print(ret[-5:])
    if ret[:-5] != b:
        print("ret != b; dumped to asdf.bin")
        with open('asdf.bin', 'wb') as f:
            f.write(ret[:-5])
        print("did not jump")
        return

    ret = sync(ser, b'JUMP', wait=0)
    args = struct.pack("<I", dest)
    ser.write(args)
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

with serial.Serial('/dev/ttyUSB0', 120192, timeout=1) as ser:
    #console(ser)
    do(ser, b)
