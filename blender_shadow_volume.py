import bpy
import bmesh
from mathutils import Vector
from collections import defaultdict
from itertools import combinations, chain

def sprint(*text):
    screen = bpy.data.screens['Scripting']
    for area in screen.areas:
        if area.type != "CONSOLE":
            continue
        override = {'screen': screen, 'area': area}
        with bpy.context.temp_override(**override):
            bpy.ops.console.scrollback_append(text=" ".join(map(str, text)))

print = sprint

def create_shadow_volume_mesh(light, o: bpy.types.Object, vertex_indices=None):
    mesh = bpy.data.meshes.new("test")

    if vertex_indices is None:
        vertex_indices = range(len(o.data.vertices))

    length = len(vertices)
    mesh.vertices.add(1 + length)
    origin = mesh.vertices[0]
    origin.co = Vector(light.location)

    mesh.edges.add(length)

    for i, v in enumerate(vertices):
        v = o.data.vertices[ix]
        world_v = Vector(o.matrix_world @ v.co)
        mesh.vertices[1 + i].co = world_v
        mesh.edges[i].vertices = [0, 1 + i]

    object = bpy.data.objects.new("test", mesh)

    bpy.context.scene.collection.objects.link(object)

def cast_ray(light, o, ix):
    v = o.data.vertices[ix]
    start = Vector(o.matrix_world @ v.co)
    ray = start - light.location
    #ray = Vector((0, 0, 0)) - light.location
    ray.normalize()
    end = start + (ray * 10)
    return start, end

def shadow_volume_mesh_rays(light, o: bpy.types.Object, loop):
    length = len(loop)
    mesh = bpy.data.meshes.new("test")
    #mesh.vertices.add(length * 2)
    #mesh.edges.add(length)

    vertices = [0] * length * 2

    for i, ix in enumerate(loop):
        start, end = cast_ray(light, o, ix)
        vertices[i * 2 + 0] = start
        vertices[i * 2 + 1] = end

    bm = bmesh.new()
    #bm.from_mesh(mesh)
    for i in range(len(loop)):
        i1 = i
        i2 = (i + 1) % len(loop)
        bm.faces.new([
            bm.verts.new(vertices[i1 * 2 + 0]), # start
            bm.verts.new(vertices[i2 * 2 + 0]), # start
            bm.verts.new(vertices[i2 * 2 + 1]), # end
            bm.verts.new(vertices[i1 * 2 + 1]), # end
        ])
    bm.to_mesh(mesh)
    bm.free()
    object = bpy.data.objects.new("test", mesh)

    bpy.context.scene.collection.objects.link(object)

def polygon_edges(l):
    for i in range(len(l)):
        j = (i + 1) % len(l)
        yield frozenset((l[i], l[j]))

def polygons_by_edge_pairs(polygons):
    pairs = defaultdict(list)
    for i, polygon in enumerate(polygons):
        for edge in polygon_edges(polygon.vertices):
            pairs[edge].append(i)
    return list(pairs.items())

def face_indicators(light, o: bpy.types.Object):
    indicators = []
    for i, normal in enumerate(o.data.polygon_normals):
        n = o.matrix_world.to_3x3() @ normal.vector
        n.normalize()
        a = o.data.polygons[i].vertices[0]
        v = o.matrix_world @ o.data.vertices[a].co
        #d = v.dot(n)
        #indicator = n.dot(light.location) + d
        indicator = n.dot(light.location - v)
        indicators.append(indicator)
    return indicators

def edge_indices(o):
    return {
        frozenset(edge.vertices): i
        for i, edge in enumerate(o.data.edges)
    }

def object_silhouette(light, o: bpy.types.Object):
    indicators = face_indicators(light, o)
    edges = []
    for edge, polygons in polygons_by_edge_pairs(o.data.polygons):
        assert len(polygons) == 2, polygons
        a, b = polygons
        if (indicators[a] > 0) != (indicators[b] > 0):
            edges.append(edge)
    assert len(set(edges)) == len(edges)
    return edges, indicators

def delete_test_objects(collection):
    for o in collection.objects:
        if o.name.startswith("test"):
            collection.objects.unlink(o)

def edge_loop(edges):
    loop = list(edges.pop())
    while True:
        for i, (a, b) in enumerate(edges):
            if a == loop[-1]:
                if b in loop:
                    return loop
                loop.append(b)
            elif b == loop[-1]:
                if a in loop:
                    return loop
                loop.append(a)
            else:
                continue
            del edges[i]
            break
        else:
            return None

def append(l, a):
    if l[0] == -1:
        assert l[1] == -1
        l[0] = a
    else:
        assert l[1] == -1
        l[1] = a

def make_list(length):
    l = []
    for i in range(length):
        ll = [-1, -1]
        l.append(ll)
    return l

def edge_loop_graph(edges, num_vertices):
    edges_by_vertices = make_list(num_vertices)
    for i, (a, b) in enumerate(edges):
        append(edges_by_vertices[a], i)
        append(edges_by_vertices[b], i)
    return edges_by_vertices

def neq(a, b, y):
    assert a != -1
    assert b != -1
    if a == y:
        assert b != y
        return b
    else:
        assert a != y
        return a

def edge_loop2_inner(edges, graph, ix, visited_edges):
    loop = []
    while True:
        loop.append(ix)
        visited_edges[ix] = True

        a, b = edges[ix]
        next_ix_a = neq(*graph[a], ix)
        next_ix_b = neq(*graph[b], ix)
        if not visited_edges[next_ix_a]:
            ix = next_ix_a
            continue
        elif not visited_edges[next_ix_b]:
            ix = next_ix_b
            continue
        else:
            break

    print("inner", loop)
    return loop

def next_unvisited(visited):
    for i, v in enumerate(visited):
        if v == False:
            return i
    return -1

def edge_loop2(edges, graph):
    visited_edges = [False] * len(edges)
    loops = []
    while True:
        start = next_unvisited(visited_edges)
        if start == -1:
            break
        loops.append(edge_loop2_inner(edges, graph, start, visited_edges))
    return loops

def edge_loops(edges):
    edges = list(edges)
    loops = []
    while edges:
        loop = edge_loop(edges)
        if loop is None:
            break
        loops.append(loop)
    return loops

def object_end_caps(light, o: bpy.types.Object, indicators):
    front = bpy.data.meshes.new("front")
    back = bpy.data.meshes.new("back")

    bm_front = bmesh.new()
    bm_back = bmesh.new()

    for i, polygon in enumerate(o.data.polygons):
        assert len(polygon.vertices) == 4
        a = o.matrix_world @ o.data.vertices[polygon.vertices[0]].co
        b = o.matrix_world @ o.data.vertices[polygon.vertices[1]].co
        c = o.matrix_world @ o.data.vertices[polygon.vertices[2]].co
        d = o.matrix_world @ o.data.vertices[polygon.vertices[3]].co
        if indicators[i] > 0:
            face = [
                bm_front.verts.new(a),
                bm_front.verts.new(b),
                bm_front.verts.new(c),
                bm_front.verts.new(d),
            ]
            bm_front.faces.new(face)
        else:
            #ray = Vector((0, 0, 0)) - light.location
            ray_a = a - light.location
            ray_a.normalize()
            ray_b = b - light.location
            ray_b.normalize()
            ray_c = c - light.location
            ray_c.normalize()
            ray_d = d - light.location
            ray_d.normalize()
            face = [
                bm_back.verts.new(a + (ray_a * 10)),
                bm_back.verts.new(b + (ray_b * 10)),
                bm_back.verts.new(c + (ray_c * 10)),
                bm_back.verts.new(d + (ray_d * 10)),
            ]
            bm_back.faces.new(face)

    bm_front.to_mesh(front)
    bm_front.free()
    object_front = bpy.data.objects.new("test_front", front)
    bpy.context.scene.collection.objects.link(object_front)

    bm_back.to_mesh(back)
    bm_back.free()
    object_back = bpy.data.objects.new("test_back", back)
    bpy.context.scene.collection.objects.link(object_back)

light = bpy.context.scene.objects['Light']
cube = bpy.context.scene.objects['Torus']

delete_test_objects(bpy.context.scene.collection)
edges, indicators = object_silhouette(light, cube)

object_end_caps(light, cube, indicators)

for loop in edge_loops(edges):
    print("loop", len(loop))
    shadow_volume_mesh_rays(light, cube, loop)

graph = edge_loop_graph(edges, len(cube.data.vertices))

"""
print(graph)
loops = edge_loop2(edges, graph)
obj = bpy.context.edit_object
bm = bmesh.from_edit_mesh(obj.data)
for loop in loops:
    for edge_ix in loop:
        edge = edges[edge_ix]
        for e in bm.edges:
            if frozenset((e.verts[0].index, e.verts[1].index)) == frozenset(edge):
                e.select = True
bmesh.update_edit_mesh(obj.data)
"""

"""
obj = bpy.context.edit_object
bm = bmesh.from_edit_mesh(obj.data)
bm.faces[4].select = True
bmesh.update_edit_mesh(obj.data)
"""
