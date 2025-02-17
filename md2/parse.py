import struct
from dataclasses import dataclass
from functools import partial

@dataclass
class MD2Header:
    ident: int                  # magic number: "IDP2"
    version: int                # version: must be 8

    skinwidth: int              # texture width
    skinheight: int             # texture height

    framesize: int              # size in bytes of a frame

    num_skins: int              # number of skins
    num_vertices: int           # number of vertices per frame
    num_st: int                 # number of texture coordinates
    num_tris: int               # number of triangles
    num_glcmds: int             # number of opengl commands
    num_frames: int             # number of frames

    offset_skins: int           # offset skin data
    offset_st: int              # offset texture coordinate data
    offset_tris: int            # offset triangle data
    offset_frames: int          # offset frame data
    offset_glcmds: int          # offset OpenGL command data
    offset_end: int             # offset end of file

@dataclass
class MD2Triangle:
    vertex: tuple[int, int, int] # vertex indices of the triangle
    st: tuple[int, int, int]     # texture coordinate indices

@dataclass
class MD2Vertex:
    v: tuple[int, int, int]      # position
    normal_index: int            # normal vector index

@dataclass
class MD2Frame:
    scale: tuple[float, float, float]
    translate: tuple[float, float, float]
    name: str
    verts: list[MD2Vertex]

@dataclass
class MD2TextureCoordinate:
    s: int
    t: int

@dataclass
class MD2File:
    header: MD2Header
    triangles: list[MD2Triangle]
    frames: list[MD2Frame]
    texture_coordinates: list[MD2TextureCoordinate]

def parse_header(mem, offset):
    size = 17*4
    fields = struct.unpack("<" + "i" * 17, mem[offset:offset+size])
    header = MD2Header(*fields)
    assert header.ident == 0x32504449, header.ident # IDP2
    assert header.version == 8, header.version
    return offset+size, header

def parse_triangle(mem, offset):
    size = 6*2
    fields = struct.unpack("<" + "H" * 6, mem[offset:offset+size])
    (v1, v2, v3, st1, st2, st3) = fields
    return offset+size, MD2Triangle(
        (v1, v2, v3),
        (st1, st2, st3),
    )

def parse_vertex(mem, offset):
    size = 4*1
    fields = struct.unpack("<" + "B" * 4, mem[offset:offset+size])
    v1, v2, v3, normal_index = fields
    return offset+size, MD2Vertex(
        (v1, v2, v3),
        normal_index
    )

def parse_multiple(mem, offset, count, parse):
    items = []
    for i in range(count):
        offset, item = parse(mem, offset)
        items.append(item)
    return offset, items

parse_triangles = partial(parse_multiple, parse=parse_triangle)
parse_vertices = partial(parse_multiple, parse=parse_vertex)

def parse_cstring(l):
    cs = []
    for b in l:
        if b == b'\x00':
            break
        cs.append(b)
    return b"".join(cs)

def parse_frame(mem, offset, num_vertices):
    vec_size = 4 * 3
    scale = struct.unpack("<fff", mem[offset:offset+vec_size])
    offset += vec_size
    translate = struct.unpack("<fff", mem[offset:offset+vec_size])
    offset += vec_size
    name = struct.unpack("<" + "c" * 16, mem[offset:offset+16])
    offset += 16
    offset, vertices = parse_vertices(mem, offset, num_vertices)
    return offset, MD2Frame(
        scale,
        translate,
        parse_cstring(name),
        vertices
    )

def parse_frames(mem, header):
    offset = header.offset_frames
    frames = []
    for i in range(header.num_frames):
        end_of_frame, frame = parse_frame(mem, offset, header.num_vertices)
        offset += header.framesize
        assert offset == end_of_frame, (offset, end_of_frame)
        frames.append(frame)
    return offset, frames

def parse_texture_coordinate(mem, offset):
    size = 2 * 2
    s, t = struct.unpack("<" + "h" * 2, mem[offset:offset+size])
    offset += size
    return offset, MD2TextureCoordinate(s, t)

parse_texture_coordinates = partial(parse_multiple, parse=parse_texture_coordinate)

def parse_md2file(mem):
    _, header = parse_header(mem, 0)
    _, triangles = parse_triangles(mem,
                                   header.offset_tris,
                                   header.num_tris)
    _, frames = parse_frames(mem, header)
    _, texture_coordinates = parse_texture_coordinates(mem,
                                                       header.offset_st,
                                                       header.num_st)
    return MD2File(
        header,
        triangles,
        frames,
        texture_coordinates
    )
