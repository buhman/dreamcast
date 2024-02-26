import csv

def as_dict(header, row0):
    row = [s.strip() for s in row0]
    return dict(zip(header, row))

def read_input(filename):
    with open(filename) as f:
        reader = csv.reader(f, delimiter=",", quotechar='"')
        header, *rows = reader

    rows = [
        as_dict(header, row)
        for row in rows
        if "".join(map(str, row)).strip()
    ]

    return rows

def read_input_headerless(filename):
    with open(filename) as f:
        reader = csv.reader(f, delimiter=",", quotechar='"')
        rows = [
            [s.strip() for s in row]
            for row in reader
        ]
    return rows
