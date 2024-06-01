import math
from dataclasses import dataclass
import sys
from typing import Union

from generate import renderer

with open(sys.argv[1], 'r') as f:
    lines = f.read().split("\n")

@dataclass(frozen=True)
class Vertex:
    x: float
    y: float
    z: float

@dataclass(frozen=True)
class Color:
    r: float
    g: float
    b: float

@dataclass(frozen=True)
class TextureCoordinate:
    u: float
    v: float

@dataclass
class VertexIx:
    vertex: int

@dataclass
class VertexNormal:
    vertex: int
    normal: int

@dataclass
class VertexTextureNormal:
    vertex: int
    texture: int
    normal: int

Face = Union[tuple[VertexTextureNormal, VertexTextureNormal, VertexTextureNormal],
             tuple[VertexNormal, VertexNormal, VertexNormal],
             tuple[VertexIx, VertexIx, VertexIx]]

name = None

def parse_object(line):
    h, name = line.split()
    assert h == 'o'
    return name.lower()

def parse_vertex(line):
    h, *xyz_rgb = line.split()
    assert h == 'v' or h == 'vn'
    if h == 'vn':
        assert len(xyz_rgb) == 3
    if h == 'v':
        assert len(xyz_rgb) == 6 or len(xyz_rgb) == 3
    coords = list(map(float, xyz_rgb))
    if len(xyz_rgb) == 6:
        return Vertex(*coords[0:3]), Color(*coords[3:6])
    else:
        return Vertex(*coords[0:3])

def parse_texture_coordinate(line):
    h, *uv = line.split()
    assert h == 'vt'
    assert len(uv) == 2
    return TextureCoordinate(*map(float, uv))

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
        if '/' in ixs:
            ix = ixs.split('/')
            assert len(ix) == 3
            vertex_ix, uv_ix, normal_ix = [
                maybe_int(iix, offset=-1)
                for iix in ix
            ]
            assert vertex_ix is not None
            assert normal_ix is not None
            if uv_ix is None:
                return VertexNormal(vertex_ix, normal_ix)
            else:
                return VertexTextureNormal(vertex_ix, uv_ix, normal_ix)
        else:
            return VertexIx(int(ixs))

    return tuple(map(parse_ixs, tri))

def vertex_type(vertices):
    types = set(type(v) for v in vertices)
    assert len(types) == 1, types
    if type(vertices[0]) is tuple:
        return "position__color"
    elif type(vertices[0]) is Vertex:
        return "vec3"
    else:
        assert False, type(verticies[0])

def generate_vertices(vertices):
    type_str = vertex_type(vertices)
    yield f"constexpr {type_str} vertices[] = {{"
    for p_c in vertices:
        if type(p_c) is tuple:
            p, c = p_c
            yield f"{{ {{{p.x:9f}f, {p.y:9f}f, {p.z:9f}f}}, {{{c.r:9f}f, {c.g:9f}f, {c.b:9f}f}} }},"
        else:
            assert type(p_c) is Vertex
            p = p_c
            yield f"{{ {p.x:9f}f, {p.y:9f}f, {p.z:9f}f }},"
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

def face_type_str(face_type):
    if face_type is VertexIx:
        return "face_v"
    elif face_type is VertexNormal:
        return "face_vn"
    elif face_type is VertexTextureNormal:
        return "face_vtn"
    else:
        assert False, face_type

def generate_faces(faces, face_type):
    def face_coords(vtn):
        if face_type is VertexIx:
            return [vtn.vertex - 1]
        elif face_type is VertexNormal:
            return [vtn.vertex - 1, vtn.normal - 1]
        elif face_type is VertexTextureNormal:
            return [vtn.vertex - 1, vtn.texture - 1, vtn.normal - 1]
        else:
            assert False, face_type
    max_ix = max(
        i
        for f in faces
        for vtn in f
        for i in face_coords(vtn)
    )
    align = 1 + math.floor(math.log(max_ix) / math.log(10))
    type_str = face_type_str(face_type)
    yield f"constexpr {type_str} faces[] = {{"
    def align_vtn(vtn):
        return ", ".join(str(ix).rjust(align) for ix in face_coords(vtn))
    for f in faces:
        inner = ", ".join(f"{{{align_vtn(vtn)}}}" for vtn in f)
        yield f"{{{inner}}},"
    yield "};"
    yield ""
    yield f"constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof ({type_str}));"
    yield ""

def generate_namespace(vertices, texture_coordinates, normals, faces, face_type):
    global name
    assert name is not None
    yield "#pragma once"
    yield ""
    yield '#include "geometry/geometry.hpp"'
    yield ""
    yield f"namespace {name} {{"

    yield from generate_vertices(vertices)
    if texture_coordinates != []:
        yield from generate_texture_coordinates(texture_coordinates)
    if normals != []:
        yield from generate_normals(normals)
    yield from generate_faces(faces, face_type)

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

    face_types = set(type(vtn) for f in faces for vtn in f)
    assert len(face_types) == 1, face_types
    face_type = next(iter(face_types))
    if face_type is VertexTextureNormal:
        texture_coordinates, faces = merge_texture_coordinates(texture_coordinates, faces)

    render, out = renderer()
    render(generate_namespace(vertices, texture_coordinates, normals, faces, face_type))
    sys.stdout.write(out.getvalue())

if __name__ == '__main__':
    main()
