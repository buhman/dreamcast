import sys

from generate import renderer
from csv_input import read_input_headerless
from generic_sparse_struct import parse
from generic_sparse_struct import headers
from generic_sparse_struct import render_declarations

_field_types = {
    "parameter_control_word": "uint32_t",
    "user_clip_": "uint32_t",
    "object_pointer": "uint32_t",
    "bounding_box_": "uint32_t",
    "isp_tsp_instruction_word": "uint32_t",
    "tsp_instruction_word": "uint32_t",
    "texture_control_word": "uint32_t",
    "data_size_for_sort_dma": "uint32_t",
    "next_address_for_sort_dma": "uint32_t",
    "face_color_": "float",
    "face_offset_color_": "float",
    "x": "float",
    "y": "float",
    "z": "float",
    "base_color_": "float",
    "base_color_0": "uint32_t",
    "base_color_1": "uint32_t",
    "offset_color_0": "uint32_t",
    "offset_color_1": "uint32_t",
    "base_intensity_": "uint32_t",
    "u": "float",
    "v": "float",
    "u_v": "uint32_t",
    "base_color": "uint32_t",
    "offset_color": "uint32_t",
    "offset_color_": "float",
    "base_intensity": "float",
    "offset_intensity": "float",
    "a_": "float",
    "b_": "float",
    "c_": "float",
    "d_": "float",
    "a_u_a_v": "uint32_t",
    "b_u_b_v": "uint32_t",
    "c_u_c_v": "uint32_t",
    "_res": "uint32_t"
}

def get_type(field_name: str):
    match = None
    match_len = 0
    for name, type in _field_types.items():
        if field_name.startswith(name) and len(name) >= match_len:
            match = type
            assert match_len != len(name), (name, match)
            match_len = len(name)
    assert match != None, field_name
    return match

if __name__ == "__main__":
    rows = read_input_headerless(sys.argv[1])
    namespace = sys.argv[2]
    declarations = parse(rows,
                         expected_offset=4,
                         expected_sizes={32, 64})
    render, out = renderer()
    render(headers())
    render(render_declarations(namespace, declarations, get_type))
    print(out.getvalue())
