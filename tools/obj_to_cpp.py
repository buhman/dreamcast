from dataclasses import dataclass
import sys

from generate import renderer

with open(sys.argv[1], 'r') as f:
    lines = f.read().split("\n")

@dataclass
class Vertex:
    x: float
    y: float
    z: float

@dataclass
class VertexNormal:
    vertex: int
    normal: int

Face = tuple[VertexNormal, VertexNormal, VertexNormal]

name = None
vertices = []
normals = []
faces = []

def parse_object(line):
    h, name = line.split()
    assert h == 'o'
    return name.lower()

def parse_vertex(line):
    h, *xyz = line.split()
    assert h == 'v' or h == 'vn'
    assert len(xyz) == 3
    return Vertex(*map(float, xyz))

def maybe_int(i, offset):
    if i.strip() == "":
        return None
    else:
        return int(i) + offset

def parse_face(line):
    h, *tri = line.split()
    assert h == 'f'
    assert len(tri) == 3
    def parse_ixs(ixs):
        ix = ixs.split('/')
        assert len(ix) == 3
        vertex_ix, uv_ix, normal_ix = [
            maybe_int(iix, offset=-1)
            for iix in ix
        ]
        return VertexNormal(vertex_ix, normal_ix)

    return tuple(map(parse_ixs, tri))

def generate_vertices(vertices):
    yield "constexpr vec3 vertices[] = {"
    for v in vertices:
        yield f"{{ {v.x:9f}f, {v.y:9f}f, {v.z:9f}f }},"
    yield "};"
    yield ""

def generate_normals(normals):
    yield "constexpr vec3 normals[] = {"
    for n in normals:
        yield f"{{ {n.x:9f}f, {n.y:9f}f, {n.z:9f}f }},"
    yield "};"
    yield ""

def generate_faces(faces):
    yield "constexpr face faces[] = {"
    for f in faces:
        inner = ", ".join(f"{{{vn.vertex:2}, {vn.normal:2}}}" for vn in f)
        yield f"{{{inner}}},"
    yield "};"
    yield ""
    yield "constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face));"
    yield ""

for line in lines:
    if line.startswith('o '):
        assert name is None
        name = parse_object(line)
    elif line.startswith('v '):
        vertices.append(parse_vertex(line))
    elif line.startswith('vn '):
        normals.append(parse_vertex(line))
    elif line.startswith('f '):
        faces.append(parse_face(line))
    else:
        pass

def generate_namespace():
    assert name is not None
    yield "#pragma once"
    yield ""
    yield '#include "geometry.hpp"'
    yield ""
    yield f"namespace {name} {{"

    yield from generate_vertices(vertices)
    yield from generate_normals(normals)
    yield from generate_faces(faces)

    yield "}"

render, out = renderer()
render(generate_namespace())
sys.stdout.write(out.getvalue())
