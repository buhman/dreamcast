import sys
from dataclasses import dataclass, field
from pprint import pprint

@dataclass
class MD5MeshJoint:
    bone_name: str = None
    parent_index: int = None
    x_pos: float = None
    y_pos: float = None
    z_pos: float = None
    x_orient: float = None
    y_orient: float = None
    z_orient: float = None

@dataclass
class MD5MeshVert:
    vert_index: int = None
    tex_u: float = None
    tex_v: float = None
    weight_index: int = None
    weight_elem: int = None

@dataclass
class MD5MeshTri:
    tri_index: int = None
    vert_index1: int = None
    vert_index2: int = None
    vert_index3: int = None

@dataclass
class MD5MeshWeight:
    weight_index: int = None
    joint_index: int = None
    weight_value: float = None
    x_pos: float = None
    y_pos: float = None
    z_pos: float = None

@dataclass
class MD5MeshMesh:
    mesh_name: str = None
    shader: str = None
    num_verts: int = None
    verts: list[MD5MeshVert] = field(default_factory=lambda: list())
    num_tris: int = None
    tris: list[MD5MeshTri] = field(default_factory=lambda: list())
    num_weights: int = None
    weights: list[MD5MeshWeight] = field(default_factory=lambda: list())

@dataclass
class MD5Mesh:
    num_joints: int = None
    num_meshes: int = None
    joints: list[MD5MeshJoint] = field(default_factory=lambda: list())
    meshes: list[MD5MeshMesh] = field(default_factory=lambda: list())

def parse_header(l, ix, md5mesh):
    assert l[ix+0] == "MD5Version 10"
    assert l[ix+1].startswith("commandline")
    return ix + 2

def parse_parameters(l, ix, md5mesh):
    assert l[ix+0].startswith("numJoints ")
    assert l[ix+1].startswith("numMeshes ")

    md5mesh.num_joints = int(l[ix+0].removeprefix("numJoints "), 10)
    md5mesh.num_meshes = int(l[ix+1].removeprefix("numMeshes "), 10)

    return ix + 2

def parse_joint(l, ix, md5mesh):
    s = l[ix]
    lc = s.split("//", maxsplit=1)
    if len(lc) == 2:
        line, comment = lc
    elif len(lc) == 1:
        line = lc
        comment = None
    else:
        assert False, len(lc)

    tokens = line.split()
    joint = MD5MeshJoint()

    # bone name
    bone_name = tokens[0]
    assert bone_name.startswith('"') and bone_name.endswith('"')
    joint.bone_name = bone_name[1:-1]

    # parent index
    joint.parent_index = int(tokens[1], 10)

    assert tokens[2] == "("

    joint.x_pos = float(tokens[3])
    joint.y_pos = float(tokens[4])
    joint.z_pos = float(tokens[5])

    assert tokens[6] == ")"

    assert tokens[7] == "("

    joint.x_orient = float(tokens[8])
    joint.y_orient = float(tokens[9])
    joint.z_orient = float(tokens[10])

    assert tokens[11] == ")"

    md5mesh.joints.append(joint)

    return ix + 1

def parse_joints(l, ix, md5mesh):
    assert md5mesh.joints == []

    while l[ix] != "}":
        ix = parse_joint(l, ix, md5mesh)

    return ix

def parse_mesh_vert(line, mesh):
    vert = MD5MeshVert()
    tokens = line.split()
    assert tokens[0] == "vert"
    vert.vert_index = int(tokens[1], 10)
    assert tokens[2] == "("
    vert.tex_u = float(tokens[3])
    vert.tex_v = float(tokens[4])
    assert tokens[5] == ")"
    vert.weight_index = int(tokens[6], 10)
    vert.weight_elem = int(tokens[7], 10)

    assert vert.vert_index == len(mesh.verts)
    mesh.verts.append(vert)

def parse_mesh_tri(line, mesh):
    tri = MD5MeshTri()
    tokens = line.split()
    assert tokens[0] == "tri"
    tri.tri_index = int(tokens[1], 10)
    tri.vert_index1 = int(tokens[2], 10)
    tri.vert_index2 = int(tokens[3], 10)
    tri.vert_index3 = int(tokens[4], 10)

    assert tri.tri_index == len(mesh.tris)
    mesh.tris.append(tri)

def parse_mesh_weight(line, mesh):
    weight = MD5MeshWeight()
    tokens = line.split()
    assert tokens[0] == "weight"
    weight.weight_index = int(tokens[1], 10)
    weight.joint_index = int(tokens[2], 10)
    weight.weight_value = float(tokens[3])
    assert tokens[4] == "("
    weight.x_pos = float(tokens[5])
    weight.y_pos = float(tokens[6])
    weight.z_pos = float(tokens[7])
    assert tokens[8] == ")"

    assert weight.weight_index == len(mesh.weights)
    mesh.weights.append(weight)

def parse_mesh(l, ix, md5mesh):
    mesh = MD5MeshMesh()

    while l[ix] != "}":
        line = l[ix]
        if line.startswith("shader"):
            assert mesh.shader is None
            _, shader = line.split()
            assert shader.startswith('"') and shader.endswith('"')
            mesh.shader = shader[1:-1]

        elif line.startswith("numverts"):
            assert mesh.num_verts is None
            mesh.num_verts = int(line.removeprefix("numverts "), 10)

        elif line.startswith("numtris"):
            assert mesh.num_tris is None
            mesh.num_tris = int(line.removeprefix("numtris "), 10)

        elif line.startswith("numweights"):
            assert mesh.num_weights is None
            mesh.num_weights = int(line.removeprefix("numweights "), 10)

        elif line.startswith("vert"):
            parse_mesh_vert(line, mesh)

        elif line.startswith("tri"):
            parse_mesh_tri(line, mesh)

        elif line.startswith("weight"):
            parse_mesh_weight(line, mesh)

        else:
            assert False, line

        ix += 1

    assert mesh.num_verts == len(mesh.verts)
    assert mesh.num_tris == len(mesh.tris)
    assert mesh.num_weights == len(mesh.weights)

    md5mesh.meshes.append(mesh)

    return ix

def parse_ordered_list(l, ix, md5mesh):
    assert l[ix].endswith("{"), l[ix]
    string, _ = l[ix].split()
    ix += 1

    if string == "joints":
        ix = parse_joints(l, ix, md5mesh)
    elif string == "mesh":
        ix = parse_mesh(l, ix, md5mesh)
    else:
        assert False, string

    assert l[ix] == "}", l[ix]
    ix += 1
    return ix

def parse_file(l):
    ix = 0
    md5mesh = MD5Mesh()
    ix = parse_header(l, ix, md5mesh)
    ix = parse_parameters(l, ix, md5mesh)
    while ix < len(l):
        ix = parse_ordered_list(l, ix, md5mesh)

    return md5mesh

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        buf = f.read()
    l = [i.strip() for i in buf.split('\n') if i.strip()]
    md5mesh = parse_file(l)
    pprint(md5mesh)
