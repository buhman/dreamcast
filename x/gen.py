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

visitors = None
def visit(func, obj):
    global visitors
    yield from visitors[obj_type(obj)](func, obj)

def visit_none(func, obj):
    return
    yield # empty generator

def visit_objects(func, obj):
    yield from func(obj)
    for o in obj_value(obj).objects:
        yield from visit(func, o)

def visit_self(func, obj):
    yield from func(obj)

tagged = {
    templates.Header,
    templates.Material,
    templates.TextureFilename,
    templates.Frame,
    templates.FrameTransformMatrix,
    templates.Mesh,
    templates.MeshMaterialList,
    templates.MeshNormals,
    templates.MeshTextureCoords,
    templates.AnimationKey,
    templates.AnimationOptions,
    templates.Animation,
    templates.AnimationSet,
}

visitors = {
    templates.ColorRGBA            : visit_self,
    templates.ColorRGB             : visit_self,
    templates.Matrix4x4            : visit_self,
    templates.Vector               : visit_self,
    templates.MeshFace             : visit_self,
    templates.Coords2D             : visit_self,
    templates.Reference            : visit_none,
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

name_map = {}
obj_map = {}
def add_name_map(obj):
    if type(obj) is not tuple:
        return

    name, obj = obj
    assert name not in name_map, name
    name_map[id(obj)] = name.decode("utf-8")
    obj_map[name] = obj
    yield None

type_counter = Counter()
obj_names = {}
def name_gen(obj):
    global type_counter
    global obj_names

    assert type(obj) is not tuple

    if id(obj) in name_map:
        return name_map[id(obj)]
    elif id(obj) in obj_names:
        return obj_names[id(obj)]
    else:
        i = type_counter[obj_type(obj)]
        type_counter[obj_type(obj)] += 1
        obj_names[id(obj)] = i
        return i

def get_obj_name(obj):
    type_name = type_map[obj_type(obj)]
    if type(obj) is tuple:
        name, _ = obj
        string_name = name.decode('utf-8')
    else:
        string_name = name_gen(obj)
    return f"{type_name}_{string_name}"

def type_declaration(obj):
    type_name = type_map[obj_type(obj)]
    string_name = get_obj_name(obj)
    return f"const {type_name} {string_name}"

def generate_vec3(name, name2, vertices):
    yield f"vec3 {name}_{name2}[] = {{"
    for v in vertices:
        coordinates = f"{v.x}, {v.y}, {v.z}"
        yield f"{{{coordinates}}},"
    yield "};"

def generate_faces(name, faces):
    yield f"struct mesh_face {name}_faces[] = {{"
    for f in faces:
        assert f.nFaceVertexIndices == 3
        indices = f"{', '.join(map(str, f.faceVertexIndices))}"
        yield f"{{{indices}}},"
    yield "};"

def generate_face_indices(name, face_indices):
    yield f"int {name}_face_indices[] = {{"
    for i in face_indices:
        yield f"{i},"
    yield "};"

def generate_predeclaration(obj):
    name = get_obj_name(obj)
    if obj_type(obj) is templates.Mesh:
        yield from generate_vec3(name, "vertices", obj_value(obj).vertices)
        yield from generate_faces(name, obj_value(obj).faces)
    elif obj_type(obj) is templates.MeshMaterialList:
        yield from generate_face_indices(name, obj_value(obj).faceIndices)
    elif obj_type(obj) is templates.MeshNormals:
        yield from generate_vec3(name, "normals", obj_value(obj).normals)

    yield f"extern {type_declaration(obj)};"

def generate_header(obj):
    yield f".major = {obj.major},"
    yield f".minor = {obj.minor},"
    yield f".flags = {obj.flags},"

def generate_material(obj):
    face_color = f"{obj.faceColor.r}f, {obj.faceColor.g}f, {obj.faceColor.b}f, {obj.faceColor.a}f"
    specular_color = f"{obj.specularColor.r}f, {obj.specularColor.g}f, {obj.specularColor.b}f"
    emissive_color = f"{obj.emissiveColor.r}f, {obj.emissiveColor.g}f, {obj.emissiveColor.b}f"
    yield f".face_color = {{{face_color}}},"
    yield f".power = {obj.power}f,"
    yield f".specular_color = {{{specular_color}}},"
    yield f".emissive_color = {{{emissive_color}}},"

def generate_texture_filename(obj):
    yield ""

def generate_frame(obj):
    return
    yield # empty generator function

def generate_frame_transform_matrix(obj):
    yield ".frame_matrix = {"
    m = obj.frameMatrix.v
    # transpose matrix from column-major to row-major
    yield f"{m[0]}, {m[4]}, {m[8]}, {m[12]},"
    yield f"{m[1]}, {m[5]}, {m[9]}, {m[13]},"
    yield f"{m[2]}, {m[6]}, {m[10]}, {m[14]},"
    yield f"{m[3]}, {m[7]}, {m[11]}, {m[15]},"
    yield "},"

def generate_mesh(obj):
    name = get_obj_name(obj)
    yield f".n_vertices = {obj.nVertices},"
    yield f".vertices = {name}_vertices,"
    yield f".n_faces = {obj.nFaces},"
    yield f".faces = {name}_faces,"

def generate_mesh_material_list(obj):
    name = get_obj_name(obj)
    yield f".n_materials = {obj.nMaterials},"
    yield f".n_face_indices = {obj.nFaceIndices},"
    yield f".face_indices = {name}_face_indices,"

def generate_mesh_normals(obj):
    name = get_obj_name(obj)
    yield f".n_normals = {obj.nNormals},"
    yield f".normals = {name}_normals,"
    yield f".n_face_normals = {obj.nFaceNormals},"

def generate_mesh_texture_coords(obj):
    yield f".n_texture_coords = {obj.nTextureCoords},"
    yield f".texture_coords = {{"
    for tc in obj.textureCoords:
        coordinates = f"{tc.u}, {tc.v}"
        yield f"{{{coordinates}}},"
    yield "},"

def generate_animation_key(obj):
    yield f".key_type = {obj.keyType},"
    yield f".n_keys = {obj.nKeys},"
    yield ".keys = {"
    yield "// not implemented"
    yield "},"

def generate_animation_options(obj):
    yield f".open_closed = {obj.openClosed},"
    yield f".position_quality = {obj.positionQuality},"

def generate_animation(obj):
    return
    yield

def generate_animation_set(obj):
    return
    yield

def generate_definition(obj):
    yield f"{type_declaration(obj)} = {{"

    if obj_type(obj) in tagged:
        yield f".tag = tag::{type_map[obj_type(obj)]},"

    if obj_type(obj) is templates.Header:
        yield from generate_header(obj_value(obj))
    elif obj_type(obj) is templates.Material:
        yield from generate_material(obj_value(obj))
    elif obj_type(obj) is templates.TextureFilename:
        yield from generate_texture_filename(obj_value(obj))
    elif obj_type(obj) is templates.Frame:
        yield from generate_frame(obj_value(obj))
    elif obj_type(obj) is templates.FrameTransformMatrix:
        yield from generate_frame_transform_matrix(obj_value(obj))
    elif obj_type(obj) is templates.Mesh:
        yield from generate_mesh(obj_value(obj))
    elif obj_type(obj) is templates.MeshMaterialList:
        yield from generate_mesh_material_list(obj_value(obj))
    elif obj_type(obj) is templates.MeshNormals:
        yield from generate_mesh_normals(obj_value(obj))
    elif obj_type(obj) is templates.MeshTextureCoords:
        yield from generate_mesh_texture_coords(obj_value(obj))
    elif obj_type(obj) is templates.AnimationKey:
        yield from generate_animation_key(obj_value(obj))
    elif obj_type(obj) is templates.AnimationOptions:
        yield from generate_animation_options(obj_value(obj))
    elif obj_type(obj) is templates.Animation:
        yield from generate_animation(obj_value(obj))
    elif obj_type(obj) is templates.AnimationSet:
        yield from generate_animation_set(obj_value(obj))
    elif obj_type(obj) is templates.Reference:
        # do nothing
        pass
    else:
        assert False, (obj_type(obj), obj)

    if visitors[obj_type(obj)] is visit_objects:
        yield ""
        yield ".objects = {"
        for o in obj_value(obj).objects:
            if obj_type(o) == templates.Reference:
                reference_name = obj_value(o).name
                o = obj_map[reference_name]
            yield f"reinterpret_cast<const data_object *>(&{get_obj_name(o)}),"
        # end of objects list
        yield f"nullptr,"
        yield "}"

    yield "};"

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
render(gen(objects))
print(out.getvalue())
