import sys
from dataclasses import dataclass
from os import path

def parse_bp(s):
    if ' to ' in s:
        start0, end0 = s.split(' to ')
        if '(' in end0 and ')' in end0 and 'LEN_' in end0:
            end0 = int(start0) - 1
            start, end = int(start0), int(end0)
            return start, end
        else:
            start, end = int(start0), int(end0)
            assert start <= end, (start, end)
            return start, end
    else:
        start = int(s)
        return start, start

def bp_range(start, end):
    return set(range(start, end+1))

reserved = 0
def sanitize_field_name(name):
    global reserved
    if name == "(Reserved for future standardization)" or name == "Unused field":
        reserved += 1
        return f"_res{reserved}";
    if '(' in name:
        assert 'LEN_' in name, name
        name = name.split('(')[0].strip()

    name = name.lower().replace(' ', '_')
    return name

def sanitize_content_name(name):
    if name == 'Numerical value':
        return 'numerical_value'
    else:
        return 'bytes'

@dataclass
class Field:
    start: int
    end: int
    name: str
    content: str

def parse(rows):
    seen_bps = set()
    seen_names = set()

    for row in rows:
        start, end = parse_bp(row['BP'])
        _range = bp_range(start, end)
        assert seen_bps.intersection(_range) == set(), row
        seen_bps = seen_bps.union(_range)
        field_name = sanitize_field_name(row["Field name"])
        assert field_name not in seen_names
        seen_names.add(field_name)
        content_name = sanitize_content_name(row["Content"])

        yield Field(
            start=start,
            end=end,
            name=field_name,
            content=content_name
        )
