from dataclasses import dataclass

class EndOfInput(Exception):
    pass

def next_row(ix, rows, advance):
    if ix >= len(rows):
        raise EndOfInput

    if advance:
        while rows[ix][0] == "":
            ix += 1
            if ix >= len(rows):
                raise EndOfInput
    row = rows[ix]
    ix += 1
    return ix, row

@dataclass
class FieldDeclaration:
    offset: int
    name: str
    default: int
    array_length: str

@dataclass
class StructDeclaration:
    name: str
    fields: list[FieldDeclaration]
    size: int

def parse_type_declaration(ix, rows, expected_offset, expected_sizes):
    ix, row = next_row(ix, rows, advance=True)
    assert len(row) in {2, 3}, row
    struct_name, *empty = row
    assert all(e == "" for e in empty)
    fields = []
    last_offset = 0 - expected_offset
    res_ix = 0

    def terminate():
        size = last_offset + expected_offset
        assert size in expected_sizes, size
        return ix, StructDeclaration(
            struct_name,
            fields,
            size
        )

    seen_names = set()

    while True:
        try:
            ix, row = next_row(ix, rows, advance=False)
        except EndOfInput:
            return terminate()
        if row[0] == "":
            return terminate()
        else:
            default = None
            if len(row) == 2:
                _offset, name = row
            elif len(row) == 3:
                _offset, name, _default = row
                if _default.strip() != "":
                    default = int(_default, 16)
            else:
                assert False, row
            offset = int(_offset, 16)
            assert offset == last_offset + expected_offset, (hex(offset), hex(last_offset))
            last_offset = offset
            if name == "":
                name = f"_res{res_ix}"
                res_ix += 1

            if fields and fields[-1].name == name:
                assert offset == fields[-1].offset + (fields[-1].array_length * expected_offset)
                fields[-1].array_length += 1
            else:
                assert name not in seen_names, row
                seen_names.add(name)
                fields.append(FieldDeclaration(offset, name, default, 1))

def parse(rows, expected_offset, expected_sizes):
    ix = 0
    declarations = []
    while True:
        try:
            ix, declaration = parse_type_declaration(ix, rows, expected_offset, expected_sizes)
        except EndOfInput:
            break
        declarations.append(declaration)

    return declarations

def render_initializer(declaration, get_type):
    initializer = f"constexpr {declaration.name}("
    padding = " " * len(initializer)
    def start(i):
        if i == 0:
            return initializer
        else:
            return padding

    constructor_fields = [f for f in declaration.fields
                          if (not f.name.startswith('_res')
                              and f.default is None
                              )]
    for i, field in enumerate(constructor_fields):
        s = start(i)
        assert field.array_length <= 4, field
        type = get_type(field.name) if field.array_length == 1 else "uint32_t"
        comma = ',' if i + 1 < len(constructor_fields) else ''
        yield s + f"const {type} {field.name}" + comma

    if constructor_fields:
        yield padding + ')'
    else:
        yield initializer + ')'

    for i, field in enumerate(declaration.fields):
        if field.array_length > 1:
            continue
        value = field.name if not field.name.startswith('_res') else '0'
        value = hex(field.default) if field.default is not None else value
        s = ':' if i == 0 else ','
        yield "  " + s + f" {field.name}({value})"

    array_fields = [f for f in declaration.fields
                    if f.array_length > 1]
    if array_fields:
        yield "{"
        for field in array_fields:
            yield f"byte_order<{field.array_length}>(&this->{field.name}[0], {field.name});"
        yield "}"
    else:
        yield "{ }"

def render_static_assertions(declaration):
    yield f"static_assert((sizeof ({declaration.name})) == {declaration.size});"
    for field in declaration.fields:
        yield f"static_assert((offsetof (struct {declaration.name}, {field.name})) == 0x{field.offset:02x});"

def render_data_method():
    yield "const uint8_t * _data()"
    yield "{"
    yield "return reinterpret_cast<const uint8_t *>(this);"
    yield "}"

def render_declaration(declaration, get_type):
    yield f"struct {declaration.name} {{"

    for field in declaration.fields:
        type = get_type(field.name)
        if field.array_length == 1:
            yield f"{type} {field.name};"
        else:
            yield f"{type} {field.name}[{field.array_length}];"
    yield ""

    yield from render_initializer(declaration, get_type)
    yield ""
    yield from render_data_method();

    yield "};" # struct {declaration.name}
    yield from render_static_assertions(declaration)

def render_declarations(namespace, declarations, get_type):
    yield f"namespace {namespace} {{"
    for declaration in declarations:
        yield from render_declaration(declaration, get_type)
        yield ""
    yield "}"

def headers():
    yield "#pragma once"
    yield ""
    yield "#include <cstdint>"
    yield "#include <cstddef>"
    yield ""
