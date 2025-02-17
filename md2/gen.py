from generate import renderer
from parse import parse_md2file
import sys

def join(l):
    return ", ".join(str(i) for i in l)

def gen_header(prefix, header):
    yield f"struct md2_header {prefix}_header = {{"

    yield f".skinwidth = {header.skinwidth},"
    yield f".skinheight = {header.skinheight},"

    yield f".num_vertices = {header.num_vertices},"
    yield f".num_st = {header.num_st},"
    yield f".num_tris = {header.num_tris},"
    yield f".num_frames = {header.num_frames},"

    yield f".st = {prefix}_st,"
    yield f".tris = {prefix}_triangle,"
    yield f".frames = {prefix}_frame,"

    yield "};"

def gen_triangles(prefix, triangles):
    yield f"struct md2_triangle {prefix}_triangle[] = {{"
    for t in triangles:
        yield "{"
        yield f".vertex = {{{join(t.vertex)}}},"
        yield f".st = {{{join(t.st)}}},"
        yield "},"
    yield "};"

def gen_frames(prefix, frames):
    yield f"struct md2_frame {prefix}_frame[] = {{"
    for i, f in enumerate(frames):
        yield "{"
        yield f".scale = {{{join(f.scale)}}},"
        yield f".translate = {{{join(f.translate)}}},"
        yield f'.name = "{f.name}",'
        yield f".verts = {prefix}_verts_{i}"
        yield "},"
    yield "};"

def gen_frame_vertices(prefix, frames):
    for i, f in enumerate(frames):
        yield f"struct md2_vertex {prefix}_verts_{i}[] = {{"
        for v in f.verts:
            yield "{"
            yield f".v = {{{join(v.v)}}},"
            yield f".normal_index = {v.normal_index},"
            yield "},"
        yield "};"

def gen_texture_coordinates(prefix, texture_coordinates):
    yield f"struct md2_texture_coordinate {prefix}_st[] = {{"
    for st in texture_coordinates:
        yield f"{{{st.s}, {st.t}}},"
    yield "};"

def generate_all(prefix, md2file):
    yield from gen_triangles(prefix, md2file.triangles)
    yield from gen_frame_vertices(prefix, md2file.frames)
    yield from gen_frames(prefix, md2file.frames)
    yield from gen_texture_coordinates(prefix, md2file.texture_coordinates)
    yield from gen_header(prefix, md2file.header)

with open(sys.argv[1], 'rb') as f:
    buf = f.read()

prefix = sys.argv[2]
output_filename = sys.argv[3]

mem = memoryview(buf)
md2file = parse_md2file(mem)

render, out = renderer()
render(generate_all(prefix, md2file))
with open(output_filename, 'w') as f:
    f.write(out.getvalue())
