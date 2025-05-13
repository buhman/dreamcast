from generate import renderer
import sys
import md5anim
from md5mesh_gen import vec2, vec3, vec4, unit_quaternion_w

def _render_md5_anim_hierarchy(h):
    yield f'.name = "{h.name}",'
    yield f".parent_index = {h.parent_index},"
    yield f".flags = {h.flags},"
    yield f".start_index = {h.start_index},"

def render_md5_anim_hierarchy(prefix, hierarchy):
    yield f"struct md5_anim_hierarchy {prefix}_hierarchy[] = {{"
    for h in hierarchy:
        yield "{"
        yield from _render_md5_anim_hierarchy(h)
        yield "},"
    yield "};"

def _render_md5_anim_bounds(b):
    min = vec3(b.min_x, b.min_y, b.min_z)
    max = vec3(b.max_x, b.max_y, b.max_z)
    yield f".min = {min},"
    yield f".max = {max},"

def render_md5_anim_bounds(prefix, bounds):
    yield f"struct md5_anim_bounds {prefix}_bounds[] = {{"
    for b in bounds:
        yield "{"
        yield from _render_md5_anim_bounds(b)
        yield "},"
    yield "};"

def _render_md5_anim_base_frame(bf):
    pos = vec3(bf.pos_x, bf.pos_x, bf.pos_x)
    w = unit_quaternion_w(bf.orient_x, bf.orient_y, bf.orient_z)
    orient = vec4(bf.orient_x, bf.orient_y, bf.orient_z, w)
    yield f".pos = {pos},"
    yield f".orient = {orient},"

def render_md5_anim_base_frame(prefix, base_frame):
    yield f"struct md5_anim_base_frame {prefix}_base_frame[] = {{"
    for bf in base_frame:
        yield "{"
        yield from _render_md5_anim_base_frame(bf)
        yield "},"
    yield "};"

def render_md5_anim_frames(prefix, frame):
    for i, anim_frame in enumerate(frame):
        yield f"float {prefix}_{i}_frame[] = {{"
        for f in anim_frame.value:
            yield f"{f:.6f},"
        yield "};"

def render_md5_anim_frame(prefix, frame):
    yield from render_md5_anim_frames(prefix, frame)
    yield f"float * {prefix}_frame[] = {{"
    for i, anim_frame in enumerate(frame):
        yield f"{prefix}_{i}_frame,"
    yield "};"

def render_md5_anim(prefix, a):
    yield f".num_frames = {a.num_frames},"
    yield f".num_joints = {a.num_joints},"
    yield f".frame_rate = {a.frame_rate},"
    yield f".num_animated_components = {a.num_animated_components},"
    yield f".hierarchy = {prefix}_hierarchy,"
    yield f".bounds = {prefix}_bounds,"
    yield f".base_frame = {prefix}_base_frame,"
    yield f".frame = {prefix}_frame,"

def render_anim(prefix, a):
    yield from render_md5_anim_hierarchy(prefix, a.hierarchy)
    yield from render_md5_anim_bounds(prefix, a.bounds)
    yield from render_md5_anim_base_frame(prefix, a.base_frame)
    yield from render_md5_anim_frame(prefix, a.frame)

    yield f"md5_anim {prefix}_anim {{"
    yield from render_md5_anim(prefix, a)
    yield "};"

def render_all(prefix, a):
    yield from render_anim(prefix, a)

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        buf = f.read()
    l = [i.strip() for i in buf.split('\n') if i.strip()]
    a = md5anim.parse_file(l)

    render, out = renderer()
    render(render_all(sys.argv[2], a))
    sys.stdout.write(out.getvalue())
