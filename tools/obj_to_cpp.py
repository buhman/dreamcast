from dataclasses import dataclass
import sys

from generate import renderer

with open(sys.argv[1], 'r') as f:
    lines = f.read().split("\n")

@dataclass(frozen=True)
class Vertex:
    x: float
    y: float
    z: float

@dataclass(frozen=True)
class TextureCoordinate:
    u: float
    v: float

@dataclass
class VertexTextureNormal:
    vertex: int
    texture: int
    normal: int

Face = tuple[VertexTextureNormal, VertexTextureNormal, VertexTextureNormal]

name = None

def parse_object(line):
    h, name = line.split()
    assert h == 'o'
    return name.lower()

def parse_vertex(line):
    h, *xyz = line.split()
    assert h == 'v' or h == 'vn'
    assert len(xyz) == 3
    return Vertex(*map(float, xyz))

def parse_texture_coordinate(line):
    h, *uv = line.split()
    assert h == 'vt'
    assert len(uv) == 2
    return TextureCoordinate(*map(float, uv))

def maybe_int(i, offset):
    if i.strip() == "":
        assert False
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
        return VertexTextureNormal(vertex_ix, uv_ix, normal_ix)

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

def generate_texture_coordinates(texture_coordinates):
    yield "constexpr vec2 texture[] = {"
    for t in texture_coordinates:
        yield f"{{ {t.u:9f}f, {t.v:9f}f }},"
    yield "};"
    yield ""

def generate_faces(faces):
    yield "constexpr face faces[] = {"
    for f in faces:
        inner = ", ".join(f"{{{vtn.vertex:3}, {vtn.texture:3}, {vtn.normal:3}}}" for vtn in f)
        yield f"{{{inner}}},"
    yield "};"
    yield ""
    yield "constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face));"
    yield ""

def generate_namespace(vertices, texture_coordinates, normals, faces):
    global name
    assert name is not None
    yield "#pragma once"
    yield ""
    yield '#include "geometry.hpp"'
    yield ""
    yield f"namespace {name} {{"

    yield from generate_vertices(vertices)
    yield from generate_texture_coordinates(texture_coordinates)
    yield from generate_normals(normals)
    yield from generate_faces(faces)

    yield "}"

def merge_texture_coordinates(texture_coordinates, faces):
    ix = 0
    _texture = dict()
    _faces = []
    for face in faces:
        for vtn in face:
            key = texture_coordinates[vtn.texture]
            if key not in _texture:
                _texture[key] = ix
                ix += 1
        _faces.append(tuple(
            VertexTextureNormal(vtn.vertex,
                                _texture[texture_coordinates[vtn.texture]],
                                vtn.normal)
            for vtn in face
        ))
    return _texture, _faces


def main():
    global name
    vertices = []
    texture_coordinates = []
    normals = []
    faces = []

    for line in lines:
        if line.startswith('o '):
            assert name is None
            name = parse_object(line)
        elif line.startswith('v '):
            vertices.append(parse_vertex(line))
        elif line.startswith('vn '):
            normals.append(parse_vertex(line))
        elif line.startswith('vt '):
            texture_coordinates.append(parse_texture_coordinate(line))
        elif line.startswith('f '):
            faces.append(parse_face(line))
        else:
            pass

    texture_coordinates, faces = merge_texture_coordinates(texture_coordinates, faces)

    render, out = renderer()
    render(generate_namespace(vertices, texture_coordinates, normals, faces))
    sys.stdout.write(out.getvalue())

if __name__ == '__main__':
    main()
