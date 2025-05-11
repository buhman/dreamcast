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
    y_orient: float = None

@dataclass
class MD5MeshVert:
    vert_index: int = None
    v

@dataclass
class MD5MeshMesh:
    mesh_name: str = None
    shader: str = None
    verts: list[MD5MeshVert] = None


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

def parse_mesh(l, ix, md5mesh):


def parse_ordered_list(l, ix, md5mesh):
    assert l[ix].endswith("{"), l[ix]
    string = l[ix].split()[0]
    ix += 1

    if string == "joints":
        ix = parse_joints(l, ix, md5mesh)
    elif string == "mesh":
        ix = parse_mesh(l, ix, md5mesh)
    else:
        assert False, string

    assert l[ix] == "}"
    ix += 1
    return ix

def parse_file(l):
    ix = 0
    md5mesh = MD5Mesh()
    ix = parse_header(l, ix, md5mesh)
    ix = parse_parameters(l, ix, md5mesh)
    while ix < len(l):
        ix = parse_ordered_list(l, ix, md5mesh)

    pprint(md5mesh)

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        buf = f.read()

    l = [i.strip() for i in buf.split('\n') if i.strip()]

    parse_file(l)
