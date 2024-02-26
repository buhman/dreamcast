import sys

from generate import renderer
from csv_input import read_input_headerless
from generic_sparse_struct import parse
from generic_sparse_struct import headers
from generic_sparse_struct import render_declarations

def get_type(field_name: str):
    return "uint8_t"

if __name__ == "__main__":
    rows = read_input_headerless(sys.argv[1])
    namespace = sys.argv[2]
    declarations = parse(rows,
                         expected_offset=1,
                         expected_sizes={12})
    from pprint import pprint
    render, out = renderer()
    render(headers())
    render(render_declarations(namespace, declarations, get_type))
    print(out.getvalue())
