import lex
import templates

class TokenReader:
    def __init__(self, buf):
        mem = memoryview(buf)
        self.tokens = list(lex.lex_all(mem, 0))
        self.ix = 0

    def consume(self, o):
        assert self.tokens[self.ix] == o, (self.tokens[self.ix], o)
        self.ix += 1

    def consume_type(self, t):
        assert type(self.tokens[self.ix]) == t, (t,
                                                 self.tokens[self.ix],
                                                 type(self.tokens[self.ix]))
        self.ix += 1
        return self.tokens[self.ix-1]

    def match(self, o):
        if self.tokens[self.ix] == o:
            self.ix += 1
            return self.tokens[self.ix-1]
        else:
            return False

    def match_type(self, t):
        if type(self.tokens[self.ix]) == t:
            self.ix += 1
            return self.tokens[self.ix-1]
        else:
            return False

    def peek(self):
        return self.tokens[self.ix]

    def eof(self):
        return self.ix >= len(self.tokens)

def parse_int_raw(r):
    i = r.consume_type(int)
    return i

def parse_float_raw(r):
    i = r.consume_type(float)
    return i

def parse_int(r):
    i = r.consume_type(int)
    r.consume(lex.TOKEN_SEMICOLON)
    return i

def parse_float(r):
    f = r.consume_type(float)
    r.consume(lex.TOKEN_SEMICOLON)
    return f

def parse_vector(r):
    x = parse_float(r)
    y = parse_float(r)
    z = parse_float(r)
    return templates.Vector(x, y, z)

def parse_string(r):
    s = r.consume_type(str)
    r.consume(lex.TOKEN_SEMICOLON)
    return s

def parse_color_rgba(r):
    red = r.consume_type(float)
    r.consume(lex.TOKEN_COMMA)
    green = r.consume_type(float)
    r.consume(lex.TOKEN_COMMA)
    blue = r.consume_type(float)
    r.consume(lex.TOKEN_COMMA)
    alpha = r.consume_type(float)
    r.consume(lex.TOKEN_SEMICOLON)
    r.consume(lex.TOKEN_SEMICOLON)
    return templates.ColorRGBA(
        red, green, blue, alpha
    )

def parse_color_rgb(r):
    red = r.consume_type(float)
    r.consume(lex.TOKEN_COMMA)
    green = r.consume_type(float)
    r.consume(lex.TOKEN_COMMA)
    blue = r.consume_type(float)
    r.consume(lex.TOKEN_SEMICOLON)
    r.consume(lex.TOKEN_SEMICOLON)
    return templates.ColorRGB(
        red, green, blue
    )

def parse_list(r, n, parse, *, delim=lex.TOKEN_COMMA):
    l = []
    for _ in range(n - 1):
        l.append(parse(r))
        if delim is not None:
            r.consume(delim)
    l.append(parse(r))
    r.consume(lex.TOKEN_SEMICOLON)
    return l

def parse_matrix4x4(r):
    v = []
    for i in range(15):
        v.append(r.consume_type(float))
        r.consume(lex.TOKEN_COMMA)
    v.append(r.consume_type(float))
    r.consume(lex.TOKEN_SEMICOLON)
    r.consume(lex.TOKEN_SEMICOLON)
    return templates.Matrix4x4(
        v
    )

def parse_header(r):
    r.consume(b"Header")
    r.consume(lex.TOKEN_LBRACKET)
    major = parse_int(r)
    minor = parse_int(r)
    flags = parse_int(r)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.Header(
        major, minor, flags
    )

def parse_material(r):
    r.consume(b"Material")
    name = r.consume_type(bytes)
    r.consume(lex.TOKEN_LBRACKET)
    faceColor = parse_color_rgba(r)
    power = parse_float(r)
    specularColor = parse_color_rgb(r)
    emissiveColor = parse_color_rgb(r)

    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r))

    return name, templates.Material(
        faceColor,
        power,
        specularColor,
        emissiveColor,
        objects
    )

def parse_texture_filename(r):
    r.consume(b"TextureFilename")
    r.consume(lex.TOKEN_LBRACKET)
    filename = parse_string(r)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.TextureFilename(
        filename
    )

def parse_frame(r):
    r.consume(b"Frame")
    name = r.consume_type(bytes)
    r.consume(lex.TOKEN_LBRACKET)
    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r))
    return name, templates.Frame(
        objects
    )

def parse_frame_transform_matrix(r):
    r.consume(b"FrameTransformMatrix")
    r.consume(lex.TOKEN_LBRACKET)
    frameMatrix = parse_matrix4x4(r)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.FrameTransformMatrix(
        frameMatrix
    )

def parse_mesh_face(r):
    nFaceVertexIndices = parse_int(r)
    faceVertexIndices = parse_list(r, nFaceVertexIndices, parse_int_raw)

    return templates.MeshFace(
        nFaceVertexIndices,
        faceVertexIndices,
    )

def parse_mesh(r):
    r.consume(b"Mesh")
    name = r.consume_type(bytes)
    r.consume(lex.TOKEN_LBRACKET)
    nVertices = parse_int(r)
    vertices = parse_list(r, nVertices, parse_vector)
    nFaces = parse_int(r)
    faces = parse_list(r, nFaces, parse_mesh_face)

    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r))

    return name, templates.Mesh(
        nVertices,
        vertices,
        nFaces,
        faces,
        objects
    )

def parse_mesh_material_list(r):
    r.consume(b"MeshMaterialList")
    r.consume(lex.TOKEN_LBRACKET)

    nMaterials = parse_int(r)
    nFaceIndices = parse_int(r)
    faceIndices = parse_list(r, nFaceIndices, parse_int, delim=None)

    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r, "Material"))

    return templates.MeshMaterialList(
        nMaterials,
        nFaceIndices,
        faceIndices,
        objects
    )

def parse_mesh_normals(r):
    r.consume(b"MeshNormals")
    r.consume(lex.TOKEN_LBRACKET)
    nNormals = parse_int(r)
    normals = parse_list(r, nNormals, parse_vector)
    nFaceNormals = parse_int(r)
    faceNormals = parse_list(r, nFaceNormals, parse_mesh_face)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.MeshNormals(
        nNormals,
        normals,
        nFaceNormals,
        faceNormals
    )

def parse_coords2d(r):
    u = parse_float(r)
    v = parse_float(r)
    return templates.Coords2D(u, v)

def parse_mesh_texture_coords(r):
    r.consume(b"MeshTextureCoords")
    r.consume(lex.TOKEN_LBRACKET)
    nTextureCoords = parse_int(r)
    textureCoords = parse_list(r, nTextureCoords, parse_coords2d)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.MeshTextureCoords(
        nTextureCoords,
        textureCoords
    )

def parse_animation(r):
    r.consume(b"Animation")
    name = r.consume_type(bytes)
    r.consume(lex.TOKEN_LBRACKET)
    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r, {b"AnimationKey", b"AnimationOptions"}))

    return name, templates.Animation(
        objects
    )

def parse_animation_set(r):
    r.consume(b"AnimationSet")
    name = r.consume_type(bytes)
    r.consume(lex.TOKEN_LBRACKET)
    objects = []
    while not r.match(lex.TOKEN_RBRACKET):
        objects.append(parse_one_ref(r, b"Animation"))

    return name, templates.AnimationSet(
        objects
    )

def parse_animation_options(r):
    openClosed = parse_int(r)
    positionQuality = parse_int(r)
    return templates.AnimationOptions(
        openClosed,
        positionQuality
    )

def parse_float_keys(r):
    nValues = parse_int(r)
    values = parse_list(r, nValues, parse_float_raw)
    r.consume(lex.TOKEN_SEMICOLON) # FIXME: is this correct for nKeys>1?
    return templates.TimedFloatKeys(
        nValues,
        values,
    )

def parse_timed_float_keys(r):
    time = parse_int(r)
    tfkeys = parse_float_keys(r)
    return templates.TimedFloatKeys(
        time,
        tfkeys,
    )

def parse_animation_key(r):
    r.consume(b"AnimationKey")
    r.consume(lex.TOKEN_LBRACKET)
    keyType = parse_int(r)
    nKeys = parse_int(r)
    keys = parse_list(r, nKeys, parse_timed_float_keys)
    r.consume(lex.TOKEN_RBRACKET)
    return templates.AnimationKey(
        keyType,
        nKeys,
        keys,
    )

def parse_one(r, peek_token=None):
    token = r.peek()
    if peek_token != None:
        if type(peek_token) is set:
            assert token in peek_token, (token, peek_token)
        else:
            assert token == peek_token, (token, peek_token)
    if token == b"Header":
        return parse_header(r)
    elif token == b"Material":
        return parse_material(r)
    elif token == b"TextureFilename":
        return parse_texture_filename(r)
    elif token == b"Frame":
        return parse_frame(r)
    elif token == b"FrameTransformMatrix":
        return parse_frame_transform_matrix(r)
    elif token == b"Mesh":
        return parse_mesh(r)
    elif token == b"MeshMaterialList":
        return parse_mesh_material_list(r)
    elif token == b"MeshNormals":
        return parse_mesh_normals(r)
    elif token == b"MeshTextureCoords":
        return parse_mesh_texture_coords(r)
    elif token == b"AnimationKey":
        return parse_animation_key(r)
    elif token == b"AnimationOptions":
        return parse_animation_options(r)
    elif token == b"Animation":
        return parse_animation(r)
    elif token == b"AnimationSet":
        return parse_animation_set(r)
    else:
        assert False, token

def parse_one_ref(r, peek_token=None):
    if r.match(lex.TOKEN_LBRACKET):
        name = r.consume_type(bytes)
        r.consume(lex.TOKEN_RBRACKET)
        return templates.Reference(
            name
        )
    else:
        token = r.peek()
        return parse_one(r, peek_token)

def parse_all(r):
    while not r.eof():
        yield parse_one_ref(r)

if __name__ == "__main__":
    from pprint import pprint
    import sys

    with open(sys.argv[1], "rb") as f:
        buf = f.read()

    r = TokenReader(buf)
    for i in parse_all(r):
        pprint(i)
