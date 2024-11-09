def gen_cmd(s):
    x = (0
    | s[0] << 0
    | s[1] << 8
    | s[2] << 16
    | s[3] << 24
    )

    x ^= (x << 13) & 0xffff_ffff
    x ^= (x >> 17) & 0xffff_ffff
    x ^= (x << 5) & 0xffff_ffff

    return x

commands = [
    "WRTE",
    "READ",
    "JUMP",
    "SPED",

    "MPLS",
]

replies = [
    "wrte",
    "read",
    "jump",
    "sped",
    "crcw",
    "crcr",

    "mpls",
    "mlcs",
]

seen = set()

for c in commands:
    x = gen_cmd(c.encode('ascii'))
    assert x not in seen
    seen.add(x)
    print(c, f"{x:08x}")


for c in replies:
    x = gen_cmd(c.encode('ascii'))
    assert x not in seen
    seen.add(x)
    print(c, f"{x:08x}")
