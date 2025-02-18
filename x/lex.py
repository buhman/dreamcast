import string

def parse_magic(mem, offset):
    magic = b"xof 0302txt 0064"
    window = bytes(mem[offset:offset+len(magic)])
    assert window == magic, window
    return offset + len(magic)

string_digits = set(ord(i) for i in string.digits)

def parse_number(mem, offset):
    mem = memoryview(mem)
    whole = []
    fraction = []
    sign = 1
    if mem[offset] == ord('-'):
        sign = -1
        offset += 1
    # whole
    while True:
        c = mem[offset]
        if c in string_digits:
            whole.append(c)
            offset += 1
        elif c == ord('.'):
            assert whole != [], chr(c)
            offset += 1
            break
        else:
            assert whole != [], chr(c)
            number = sign * int(bytes(whole))
            return offset, number
    # fraction
    while True:
        c = mem[offset]
        if c in string_digits:
            fraction.append(c)
            offset += 1
        else:
            assert fraction != [], chr(c)
            w = int(bytes(whole))
            f = int(bytes(fraction)) / (10 ** len(fraction))
            number = sign * (w + f)
            return offset, number

assert parse_number(b"1234;", 0)[1] == 1234
assert abs(parse_number(b"1234.5678;", 0)[1] - 1234.5678) < 0.0001
assert parse_number(b"-1234;", 0)[1] == -1234
assert abs(parse_number(b"-1234.5678;", 0)[1] - -1234.5678) < 0.0001

whitespace = set([ord(' '), ord('\n')])

TOKEN_SEMICOLON = type("TOKEN_SEMICOLON", (), {})
TOKEN_COMMA = type("TOKEN_COMMA", (), {})
TOKEN_LBRACKET = type("TOKEN_LBRACKET", (), {})
TOKEN_RBRACKET = type("TOKEN_RBRACKET", (), {})

identifier_start = set(map(ord, string.ascii_letters + "_"))
identifier = identifier_start | string_digits

def parse_identifier(mem, offset):
    l = []
    while True:
        c = mem[offset]
        if c in identifier:
            l.append(c)
            offset += 1
        else:
            assert l != []
            return offset, bytes(l)

def parse_string(mem, offset):
    assert mem[offset] == ord('"')
    offset += 1
    start = offset
    while mem[offset] != ord('"'):
        assert mem[offset] != ord("\n")
        offset += 1
    assert mem[offset] == ord('"')
    s = bytes(mem[start:offset]).decode("utf-8")
    offset += 1
    return offset, s

def next_token(mem, offset):
    while True:
        if offset >= len(mem):
            return offset, None
        c = mem[offset]
        if c in whitespace:
            offset += 1
        else:
            break

    if c in string_digits or c == ord('-'):
        return parse_number(mem, offset)
    elif c == ord(';'):
        return offset + 1, TOKEN_SEMICOLON
    elif c == ord(','):
        return offset + 1, TOKEN_COMMA
    elif c == ord('{'):
        return offset + 1, TOKEN_LBRACKET
    elif c == ord('}'):
        return offset + 1, TOKEN_RBRACKET
    elif c == ord('"'):
        return parse_string(mem, offset)
    elif c in identifier_start:
        return parse_identifier(mem, offset)
    else:
        assert False, chr(c)

def lex_all(mem, offset):
    offset = parse_magic(mem, offset)
    while True:
        offset, token = next_token(mem, offset)
        if token is None:
            return
        else:
            yield token
