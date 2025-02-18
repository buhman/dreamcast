import generate
from parse import parse_all, TokenReader
import templates
import dataclasses
from collections import Counter

from pprint import pprint
import sys

def obj_value(obj):
    if type(obj) is tuple:
        assert len(obj) == 2, obj
        assert type(obj[0]) == bytes, obj
        return obj[1]
    else:
        return obj

def obj_type(obj):
    return type(obj_value(obj))

def visit_objects(func, obj):
    print("vo", func)
    yield func(obj)
    for o in obj_value(obj).objects:
        yield func(o)

def visit_self(func, obj):
    print("vs", func)
    yield func(obj)

visitors = {
    templates.ColorRGBA            : visit_self,
    templates.ColorRGB             : visit_self,
    templates.Matrix4x4            : visit_self,
    templates.Vector               : visit_self,
    templates.MeshFace             : visit_self,
    templates.Coords2D             : visit_self,
    templates.Reference            : visit_self,
    templates.FloatKeys            : visit_self,
    templates.TimedFloatKeys       : visit_self,

    templates.Header               : visit_self,
    templates.Material             : visit_objects,
    templates.TextureFilename      : visit_self,
    templates.Frame                : visit_objects,
    templates.FrameTransformMatrix : visit_self,
    templates.Mesh                 : visit_objects,
    templates.MeshMaterialList     : visit_objects,
    templates.MeshNormals          : visit_self,
    templates.MeshTextureCoords    : visit_self,
    templates.AnimationKey         : visit_self,
    templates.AnimationOptions     : visit_self,
    templates.Animation            : visit_objects,
    templates.AnimationSet         : visit_objects,
}

type_map = {
    templates.ColorRGBA            : "vec4",
    templates.ColorRGB             : "vec3",
    templates.Matrix4x4            : "mat4x4",
    templates.Vector               : "vec3",
    templates.MeshFace             : "mesh_face",
    templates.Coords2D             : "vec2",
    #templates.Reference            : None,
    templates.FloatKeys            : "float_keys",
    templates.TimedFloatKeys       : "time_float_keys",

    templates.Header               : "header",
    templates.Material             : "material",
    templates.TextureFilename      : "texture_filename",
    templates.Frame                : "frame",
    templates.FrameTransformMatrix : "frame_transform_matrix",
    templates.Mesh                 : "mesh",
    templates.MeshMaterialList     : "mesh_material_list",
    templates.MeshNormals          : "mesh_normals",
    templates.MeshTextureCoords    : "mesh_texture_coords",
    templates.AnimationKey         : "animation_key",
    templates.AnimationOptions     : "animation_options",
    templates.Animation            : "animation",
    templates.AnimationSet         : "animation_set",
}

type_counter = Counter()
def name_gen(obj):
    global type_counter
    assert type(obj) is not tuple
    i = type_counter[type(obj)]
    type_counter[type(obj)] += 1
    return i

name_map = {}
def add_name_map(obj):
    if type(obj) is not tuple:
        return

    name, obj = obj
    assert name not in name_map, name
    name_map[name] = obj
    yield None

def type_declaration(obj):
    type_name = type_map[obj_type(obj)]
    if type(obj) is tuple:
        name, _ = obj
        string_name = name.decode('utf-8')
    else:
        string_name = name_gen(obj)
    return f"const {type_name} {type_name}_{string_name}"

def generate_predeclaration(obj):
    if type(obj) is not tuple:
        return

    yield f"{type_declaration(obj)};"

def generate_header(obj):
    yield f"{type_declaration(obj)} {{"
    yield "};"

def generate_definition(obj):
    if obj_type(obj) is templates.Header:
        yield from generate_header(obj)
    elif obj_type(obj) is templates.Material:
        yield from generate_material(obj)
    elif obj_type(obj) is templates.TextureFilename:
        yield from generate_texture_filename(obj)
    elif obj_type(obj) is templates.Frame:
        yield from generate_frame(obj)
    elif obj_type(obj) is templates.FrameTransformMatrix:
        yield from generate_frame_transform_matrix(obj)
    elif obj_type(obj) is templates.Mesh:
        yield from generate_mesh(obj)
    elif obj_type(obj) is templates.MeshMaterialList:
        yield from generate_mesh_material_list(obj)
    elif obj_type(obj) is templates.MeshNormals:
        yield from generate_mesh_normals(obj)
    elif obj_type(obj) is templates.MeshTextureCoords:
        yield from generate_mesh_texture_coords(obj)
    elif obj_type(obj) is templates.AnimationKey:
        yield from generate_animation_key(obj)
    elif obj_type(obj) is templates.AnimationOptions:
        yield from generate_animation_options(obj)
    elif obj_type(obj) is templates.Animation:
        yield from generate_animation(obj)
    elif obj_type(obj) is templates.AnimationSet:
        yield from generate_animation_set(obj)
    else:
        assert False, (type(obj), obj)

def visit(func, obj):
    yield from visitors[obj_type(obj)](func, obj)

def visit_all(func, objects):
    for obj in objects:
        yield from visit(func, obj)

def gen(objects):
    yield from visit_all(generate_predeclaration, objects)
    yield from visit_all(generate_definition, objects)

with open(sys.argv[1], "rb") as f:
    buf = f.read()
objects = list(parse_all(TokenReader(buf)))

_ = list(visit_all(add_name_map, objects))

render, out = generate.renderer()
for i in gen(objects):
    print("line", i)
print(out.getvalue())
