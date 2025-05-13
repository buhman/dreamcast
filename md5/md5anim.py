import sys
from dataclasses import dataclass, field
from pprint import pprint

@dataclass
class MD5AnimFrame:
    value: list[float] = field(default_factory=lambda: list())

@dataclass
class MD5AnimBaseFrame:
    pos_x: float = None
    pos_y: float = None
    pos_z: float = None
    orient_x: float = None
    orient_y: float = None
    orient_z: float = None

@dataclass
class MD5AnimBounds:
    min_x: float = None
    min_y: float = None
    min_z: float = None
    max_x: float = None
    max_y: float = None
    max_z: float = None

@dataclass
class MD5AnimHierarchy:
    name: str = None
    parent_index: int = None
    flags: int = None
    start_index: int = None

@dataclass
class MD5Anim:
    num_frames: int = None
    num_joints: int = None
    frame_rate: int = None
    num_animated_components: int = None
    hierarchy: list[MD5AnimHierarchy] = field(default_factory=lambda: list())
    bounds: list[MD5AnimBounds] = field(default_factory=lambda: list())
    base_frame: list[MD5AnimBaseFrame] = field(default_factory=lambda: list())
    frame: list[MD5AnimFrame] = field(default_factory=lambda: list())

def parse_header(l, ix, md5anim):
    assert l[ix+0] == "MD5Version 10"
    assert l[ix+1].startswith("commandline")
    assert l[ix+2].startswith("numFrames ")
    assert l[ix+3].startswith("numJoints ")
    assert l[ix+4].startswith("frameRate ")
    assert l[ix+5].startswith("numAnimatedComponents ")

    md5anim.num_frames = int(l[ix+2].removeprefix("numFrames "), 10)
    md5anim.num_joints = int(l[ix+3].removeprefix("numJoints "), 10)
    md5anim.frame_rate = int(l[ix+4].removeprefix("frameRate "), 10)
    md5anim.num_animated_components = int(l[ix+5].removeprefix("numAnimatedComponents "), 10)

    return ix + 6

def parse_hierarchy(l, ix, md5anim):
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
    hierarchy = MD5AnimHierarchy()

    # name
    name = tokens[0]
    assert name.startswith('"') and name.endswith('"')
    hierarchy.name = name[1:-1]

    hierarchy.parent_index = int(tokens[1], 10)
    hierarchy.flags = int(tokens[2], 10)
    hierarchy.start_index = int(tokens[3], 10)

    md5anim.hierarchy.append(hierarchy)

    return ix + 1

def parse_bounds(l, ix, md5anim):
    line = l[ix]
    tokens = line.split()

    bounds = MD5AnimBounds()

    assert tokens[0] == "("
    bounds.min_x = float(tokens[1])
    bounds.min_y = float(tokens[2])
    bounds.min_z = float(tokens[3])
    assert tokens[4] == ")"
    assert tokens[5] == "("
    bounds.max_x = float(tokens[6])
    bounds.max_y = float(tokens[7])
    bounds.max_z = float(tokens[8])
    assert tokens[9] == ")"

    md5anim.bounds.append(bounds)

    return ix + 1

def parse_base_frame(l, ix, md5anim):
    line = l[ix]
    tokens = line.split()

    base_frame = MD5AnimBaseFrame()

    assert tokens[0] == "("
    base_frame.pos_x = float(tokens[1])
    base_frame.pos_y = float(tokens[2])
    base_frame.pos_z = float(tokens[3])
    assert tokens[4] == ")"
    assert tokens[5] == "("
    base_frame.orient_x = float(tokens[6])
    base_frame.orient_y = float(tokens[7])
    base_frame.orient_z = float(tokens[8])
    assert tokens[9] == ")"

    md5anim.base_frame.append(base_frame)

    return ix + 1

def parse_frame(l, ix, md5anim):
    frame = MD5AnimFrame()

    while l[ix] != "}":
        tokens = l[ix].split()
        for token in tokens:
            value = float(token)
            frame.value.append(value)
        ix += 1

    assert len(frame.value) == md5anim.num_animated_components

    md5anim.frame.append(frame)

    return ix

def parse_ordered_list(l, ix, md5anim):
    assert l[ix].endswith("{"), l[ix]
    string, _ = l[ix].rsplit(maxsplit=1)
    ix += 1

    if string == "hierarchy":
        while l[ix] != "}":
            ix = parse_hierarchy(l, ix, md5anim)
    elif string == "bounds":
        while l[ix] != "}":
            ix = parse_bounds(l, ix, md5anim)
    elif string == "baseframe":
        while l[ix] != "}":
            ix = parse_base_frame(l, ix, md5anim)
    elif string.startswith("frame"):
        frame, frame_ix = string.split()
        assert int(frame_ix) == len(md5anim.frame)
        while l[ix] != "}":
            ix = parse_frame(l, ix, md5anim)
    assert l[ix] == "}", l[ix]
    ix += 1
    return ix

def parse_file(l):
    ix = 0
    md5anim = MD5Anim()
    ix = parse_header(l, ix, md5anim)
    while ix < len(l):
        ix = parse_ordered_list(l, ix, md5anim)

    assert len(md5anim.hierarchy) == md5anim.num_joints, (len(md5anim.hierarchy))
    assert len(md5anim.bounds) == md5anim.num_frames, (len(md5anim.bounds))
    assert len(md5anim.base_frame) == md5anim.num_joints, (len(md5anim.base_frame))
    assert len(md5anim.frame) == md5anim.num_frames, (len(md5anim.frame))

    return md5anim

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        buf = f.read()
    l = [i.strip() for i in buf.split('\n') if i.strip()]
    md5anim = parse_file(l)
    pprint(md5anim)
