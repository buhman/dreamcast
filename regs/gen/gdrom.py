from collections import defaultdict
from dataclasses import dataclass
import sys

from sh7091 import read_input, size_to_type, headers
from generate import renderer

@dataclass
class Register:
    address: int
    name: str
    size: int
    rw: set[str]

def group_by_address(rows):
    _rw = defaultdict(set)
    _size = dict()
    _groups = defaultdict(list)

    def process_row(row):
        address = int(row["address"], 16)
        name = row["name"]
        size = int(row["size"], 10)
        rw = set(row["r/w"].upper())

        if address not in _size:
            _size[address] = size
        assert _size[address] == size, row
        assert _rw[address].intersection(rw) == set(), row
        _groups[address].append(Register(
            address=address,
            name=name,
            size=size,
            rw=rw
        ))

    _ = list(map(process_row, rows))

    return list(sorted(_groups.items(), key=lambda kv: kv[0]))

def render_groups(groups):
    next_address = 0
    reserved_num = 0

    yield "struct gdrom_if_reg {"

    for address, group in groups:
        assert address >= next_address
        if address > next_address:
            padding = address - next_address
            type = size_to_type(1)
            yield f"const {type} _pad{reserved_num}[{padding}];"
            reserved_num += 1

        if len(group) > 1:
            yield "union {"

        for reg in group:
            type = size_to_type(reg.size)
            const = "const " if reg.rw == {'R'} else ""
            yield f"{const}{type} {reg.name};"

        if len(group) > 1:
            yield "};" # end of union

        next_address = address + group[0].size

    yield "};"


    for address, group in groups:
        for reg in group:
            yield f"static_assert((offsetof (struct gdrom_if_reg, {reg.name})) == {reg.address});"

    yield ""
    yield 'extern struct gdrom_if_reg gdrom_if __asm("gdrom_if");'

if __name__ == "__main__":
    input_file = sys.argv[1]
    rows = read_input(input_file)
    groups = group_by_address(rows)
    render, out = renderer()
    render(headers())
    render(render_groups(groups))
    sys.stdout.write(out.getvalue())
