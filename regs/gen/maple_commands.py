from dataclasses import dataclass
from typing import Union
import sys

from sh7091 import read_input
from generate import renderer

@dataclass
class CommandNamespace:
    name: str
    issuing_right: set[str]
    command_code: int
    data_size: int

def command_namespace(namespace: CommandNamespace,
                      data_fields: list[tuple[str, tuple[int, str]]]):
    yield f"namespace {namespace.name} {{"
    yield f"constexpr uint32_t command_code = {hex(namespace.command_code)};"
    yield ""

    if namespace.data_size == (0, None):
        assert data_fields == []
        yield "struct data_fields {"
        yield "};"
    else:
        length, variable = namespace.data_size

        if variable is not None:
            assert variable.lower() == "n"
            yield "template <typename T>"

        yield "struct data_fields {"

        for field_name, field_size in data_fields:
            const, var = field_size
            if var is None:
                if const in {1, 2, 4}:
                    bits = const * 8
                    yield f"uint{bits}_t {field_name};"
                elif field_name == "device_id":
                    yield f"struct device_id {field_name};"
                else:
                    yield f"uint8_t {field_name}[{const}];"
            elif const == 0:
                assert var == "n"
                yield f"T {field_name};"
            else:
                yield f"uint8_t {field_name}[{const} + {var.upper()}];"

        yield "};"

        yield ""

        if variable is not None:
            assert variable == "n"
            yield f"static_assert((sizeof (struct data_fields<char[0]>)) == {length});"
        else:
            yield f"static_assert((sizeof (struct data_fields)) == {length});"

    yield "}"
    yield ""

def parse_data_size(data_size, base, multiple) -> tuple[int, str]:
    def parse_term(s):
        try:
            return int(s, base) * multiple
        except ValueError:
            div = s.split("/")
            if len(div) == 1:
                # this must be a variable
                assert multiple == 1, s
                a, = div
                return a
            elif len(div) == 2:
                # this must be a variable divided by a number
                a, b = div
                b = int(b, 10)
                assert b == multiple
                return a
            else:
                assert False, div

    _terms = data_size.split("+")
    terms = tuple(parse_term(term) for term in _terms)
    if len(terms) == 1:
        term, = terms
        if type(term) == str:
            return (0, term)
        elif type(term) == int:
            return (term, None)
        else:
            assert False, (_terms, terms)
    elif len(terms) == 2:
        assert type(terms[0]) == int
        assert type(terms[1]) == str
        return terms
    else:
        assert False, (_terms, terms)

def new_aggregator():
    last_name = None
    namespace = None
    data_fields = []
    all_names = set()

    def process_row(row):
        nonlocal last_name
        nonlocal namespace
        nonlocal data_fields

        if row["name"] == "":
            assert all(v == "" for v in row.values()), row
            return

        if row["name"] != last_name:
            if namespace is not None:
                yield namespace, data_fields
            else:
                assert data_fields == []
            assert row["name"] != ""
            last_name = row["name"]
            issuing_right = set(row["issuing_right"].split(", "))
            assert all(s in {"host", "peripheral"} for s in issuing_right), row
            assert issuing_right != set(), issuing_right
            data_size = parse_data_size(row["data_size"], 16, 4)

            namespace = CommandNamespace(
                name = row["name"],
                issuing_right = issuing_right,
                command_code = int(row["command_code"], 16),
                data_size = data_size,
            )
            data_fields = []
            # fall through

        assert last_name is None or row["name"] == last_name, (row["name"], last_name)
        if row["data_field"] != "":
            assert row["data_field_size"] != ""
            data_fields.append((
                row["data_field"],
                parse_data_size(row["data_field_size"], 10, 1)
            ))

    def terminate():
        nonlocal namespace
        nonlocal data_fields
        if namespace is not None:
            yield namespace, data_fields

    def process(rows):
        for row in rows:
            yield from process_row(row)
        yield from terminate()

    return process

def headers():
    yield "#include <cstdint>"
    yield ""
    yield "struct device_id {"
    yield "uint32_t ft;"
    yield "uint32_t fd[3];"
    yield "};"
    yield "static_assert((sizeof (struct device_id)) == 16);"

input_file = sys.argv[1]
rows = read_input(input_file)
process = new_aggregator()

render, out = renderer()
render(headers())
for namespace, data_fields in process(rows):
    render(command_namespace(namespace, data_fields))
sys.stdout.write(out.getvalue())
