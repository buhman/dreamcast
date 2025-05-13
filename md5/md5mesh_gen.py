from generate import renderer
from math import sqrt
import sys
import md5mesh
from os import path
from PIL import Image

def vec2(a, b):
    return f"{{{a:.6f}, {b:.6f}}}"

def vec3(a, b, c):
    return f"{{{a:.6f}, {b:.6f}, {c:.6f}}}"

def vec4(a, b, c, d):
    return f"{{{a:.6f}, {b:.6f}, {c:.6f}, {d:.6f}}}"

def unit_quaternion_w(x, y, z):
    t = 1.0 - (x * x) - (y * y) - (z * z);
    if t < 0.0:
        return 0.0
    else:
        return -sqrt(t)

def sanitize_name(name):
    return name.replace("-", "_").replace("/", "_").replace(".", "_")

def shader_to_data_name(shader):
    name = path.splitext(shader)[0]
    return sanitize_name(name)

def render_md5_mesh_joint(joint):
    yield f'.bone_name = "{joint.bone_name}",'
    yield f".parent_index = {joint.parent_index},"
    pos = vec3(joint.x_pos, joint.y_pos, joint.z_pos)
    yield f".pos = {pos},"
    w = unit_quaternion_w(joint.x_orient, joint.y_orient, joint.z_orient)
    orient = vec4(joint.x_orient, joint.y_orient, joint.z_orient, w)
    yield f".orient = {orient},"

def render_md5_mesh_vert(vert):
    yield f".vert_index = {vert.vert_index},"
    tex = vec2(vert.tex_u, vert.tex_v)
    yield f".tex = {tex},"
    yield f".weight_index = {vert.weight_index},"
    yield f".weight_elem = {vert.weight_elem},"

def render_md5_mesh_tri(tri):
    yield f".tri_index = {tri.tri_index},"
    vi = f"{{{tri.vert_index1}, {tri.vert_index2}, {tri.vert_index3}}}"
    yield f".vert_index = {vi},"

def render_md5_mesh_weight(weight):
    yield f".weight_index = {weight.weight_index},"
    yield f".joint_index = {weight.joint_index},"
    yield f".weight_value = {weight.weight_value},"
    pos = vec3(weight.x_pos, weight.y_pos, weight.z_pos)
    yield f".pos = {pos},"

def render_md5_mesh_mesh(prefix, i, mesh):
    name = shader_to_data_name(mesh.shader)
    yield f'.shader = &{prefix}_{name}, // {mesh.shader}'
    yield f".num_verts = {mesh.num_verts},"
    yield f".verts = {prefix}_{i}_verts,"
    yield f".num_tris = {mesh.num_tris},"
    yield f".tris = {prefix}_{i}_tris,"
    yield f".num_weights = {mesh.num_weights},"
    yield f".weights = {prefix}_{i}_weights,"

def render_md5_mesh(prefix, m):
    yield f".num_joints = {m.num_joints},"
    yield f".num_meshes = {m.num_meshes},"
    yield f".joints = {prefix}_joints,"
    yield f".meshes = {prefix}_meshes,"

def render_md5_mesh_verts(prefix, i, verts):
    yield f"struct md5_mesh_vert {prefix}_{i}_verts[] = {{"
    for vert in verts:
        yield "{"
        yield from render_md5_mesh_vert(vert)
        yield "},"
    yield "};"

def render_md5_mesh_tris(prefix, i, tris):
    yield f"struct md5_mesh_tri {prefix}_{i}_tris[] = {{"
    for tri in tris:
        yield "{"
        yield from render_md5_mesh_tri(tri)
        yield "},"
    yield "};"

def render_md5_mesh_weights(prefix, i, weights):
    yield f"struct md5_mesh_weight {prefix}_{i}_weights[] = {{"
    for weight in weights:
        yield "{"
        yield from render_md5_mesh_weight(weight)
        yield "},"
    yield "};"

def render_md5_mesh_joints(prefix, joints):
    yield f"struct md5_mesh_joint {prefix}_joints[] = {{"
    for joint in joints:
        yield "{"
        yield from render_md5_mesh_joint(joint)
        yield "},"
    yield "};"

offset = None

def render_shader(prefix, dirname, shader):
    global offset
    name = shader_to_data_name(shader)
    dir = sanitize_name(dirname)
    filename = path.join(dirname, shader)
    with Image.open(filename) as im:
        width, height = im.size

    yield f"struct md5_shader {prefix}_{name} = {{"
    yield f".start = (void *)&_binary_{dir}_{name}_data_start,"
    yield f".size = (int)&_binary_{dir}_{name}_data_size,"
    yield f".width = {width},"
    yield f".height = {height},"
    yield f".offset = {offset},"
    yield "};"

    offset += width * height * 2

def render_md5_mesh_shaders(prefix, dirname, meshes):
    global offset
    offset = 0

    shaders = set(mesh.shader for mesh in meshes)
    for shader in shaders:
        yield from render_shader(prefix, dirname, shader)
    yield f"struct md5_shader * {prefix}_shaders[] = {{"
    for shader in shaders:
        name = shader_to_data_name(shader)
        yield f"&{prefix}_{name},"
    yield "};"

def render_md5_mesh_meshes(prefix, dirname, meshes):
    for i, mesh in enumerate(meshes):
        yield from render_md5_mesh_verts(prefix, i, mesh.verts)
        yield from render_md5_mesh_tris(prefix, i, mesh.tris)
        yield from render_md5_mesh_weights(prefix, i, mesh.weights)

    yield from render_md5_mesh_shaders(prefix, dirname, meshes)

    yield f"struct md5_mesh_mesh {prefix}_meshes[] = {{"
    for i, mesh in enumerate(meshes):
        yield "{"
        yield from render_md5_mesh_mesh(prefix, i, mesh)
        yield "},"
    yield "};"

def render_mesh(prefix, dirname, m):
    yield from render_md5_mesh_joints(prefix, m.joints)
    yield from render_md5_mesh_meshes(prefix, dirname, m.meshes)

    yield f"struct md5_mesh {prefix}_mesh = {{"
    yield from render_md5_mesh(prefix, m)
    yield "};"

def render_all(prefix, dirname, m):
    yield from render_mesh(prefix, dirname, m)

if __name__ == "__main__":
    filename = sys.argv[1]
    prefix = sys.argv[2]

    dirname = path.split(filename)[0]

    with open(filename, 'r') as f:
        buf = f.read()
    l = [i.strip() for i in buf.split('\n') if i.strip()]
    m = md5mesh.parse_file(l)

    render, out = renderer()
    render(render_all(prefix, dirname, m))
    sys.stdout.write(out.getvalue())
