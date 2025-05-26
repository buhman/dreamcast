import bpy
import bmesh
from mathutils import Vector, Euler
from collections import defaultdict
from dataclasses import dataclass
import colorsys

def sprint(*text):
    screen = bpy.data.screens['Scripting']
    for area in screen.areas:
        if area.type != "CONSOLE":
            continue
        override = {'screen': screen, 'area': area}
        with bpy.context.temp_override(**override):
            bpy.ops.console.scrollback_append(text=" ".join(map(str, text)))

print = sprint

@dataclass
class Edge:
    a: int # vertex index
    b: int # vertex index

@dataclass
class PolygonIndex:
    a: int # polygon index
    b: int # polygon index

@dataclass
class EdgePolygon:
    edge: Edge
    polygon_index: PolygonIndex

@dataclass
class Graph:
    a: int # index into edge_indices
    b: int # index into edge_indices

def select_edge(bm, edge):
    for e in bm.edges:
        if frozenset((e.verts[0].index, e.verts[1].index)) == frozenset((edge.a, edge.b)):
            e.select = True

def select_edge_single(edge):
    obj = bpy.context.edit_object
    bm = bmesh.from_edit_mesh(obj.data)
    select_edge(bm, edge)
    bmesh.update_edit_mesh(obj.data)

def select_light_faces(indicators):
    obj = bpy.context.edit_object
    bm = bmesh.from_edit_mesh(obj.data)
    for i, v in enumerate(indicators):
        if v > 0:
            bm.faces[i].select = True
        else:
            bm.faces[i].select = False

    bmesh.update_edit_mesh(obj.data)

def select_silhouette(edge_polygons: list[EdgePolygon],
                      edge_indices: list[int],
                      edge_indices_length: int):
    obj = bpy.context.edit_object
    bm = bmesh.from_edit_mesh(obj.data)

    for i in range(len(edge_polygons)):
        bm.edges[i].select = False

    for i in range(edge_indices_length):
        ep = edge_polygons[edge_indices[i]]
        select_edge(bm, ep.edge)

    bmesh.update_edit_mesh(obj.data)

def face_indicators(light: Vector,
                    position: list[Vector],
                    polygon_normal: list[Vector],
                    mesh: bpy.types.Mesh,
                    # outputs
                    indicators: list[float]):
    for i in range(len(mesh.polygons)):
        n = polygon_normal[i]
        p = position[mesh.polygons[i].vertices[0]]
        indicator = n.dot(light - p)
        indicators[i] = indicator

def build_edge_polygons(mesh: bpy.types.Mesh):
    by_edge = defaultdict(list)
    for i, polygon in enumerate(mesh.polygons):
        for edge in polygon.edge_keys:
            by_edge[frozenset(edge)].append(i)

    #l = [(edge, p) for edge, p in by_edge.items() if len(p) != 2]
    #print(l[0])
    #edge, _ = l[0]
    #select_edge_single(Edge(*edge))

    assert all(len(p) == 2 for p in by_edge.values())

    return [
        EdgePolygon(Edge(*edge), PolygonIndex(*polygons))
        for edge, polygons in by_edge.items()
    ]

def object_silhouette(indicators: list[float],
                      edge_polygons: list[EdgePolygon],
                      # outputs
                      edge_indices: list[int]):
    ix = 0

    for i in range(len(edge_polygons)):
        ep = edge_polygons[i]
        if (indicators[ep.polygon_index.a] > 0) != (indicators[ep.polygon_index.b] > 0):
            edge_indices[ix] = i
            ix += 1

    return ix

def graph_append(g, v):
    assert g.a == -1 or g.b == -1
    if g.a == -1:
        g.a = v
    else:
        g.b = v

def edge_loop_graph(mesh: bpy.types.Mesh,
                    edge_polygons: list[EdgePolygon],
                    edge_indices: list[int],
                    edge_indices_length: list[int],
                    #output
                    graph: list[Graph]):
    for i in range(len(mesh.vertices)):
        graph[i].a = -1
        graph[i].b = -1

    for i in range(edge_indices_length):
        edge_index = edge_indices[i]
        edge = edge_polygons[edge_index].edge
        graph_append(graph[edge.a], i)
        graph_append(graph[edge.b], i)

def graph_next_neighbor(g: Graph,
                        ix: int):
    if g.a == ix:
        return g.b
    else:
        return g.a

def edge_loop_inner(edge_polygons: list[EdgePolygon],
                    edge_indices: list[int],
                    graph: list[Graph],
                    # output
                    visited_edge_indices: list[bool],
                    ix: int,
                    edge_loop: list[int],
                    edge_loop_ix: int):
    e = edge_polygons[edge_indices[ix]].edge
    edge_loop[edge_loop_ix] = e.b
    edge_loop_ix += 1

    while True:
        visited_edge_indices[ix] = True
        e = edge_polygons[edge_indices[ix]].edge
        next_ix_a = graph_next_neighbor(graph[e.a], ix)
        next_ix_b = graph_next_neighbor(graph[e.b], ix)

        if visited_edge_indices[next_ix_a] == False:
            edge_loop[edge_loop_ix] = e.a
            edge_loop_ix += 1
            ix = next_ix_a
        elif visited_edge_indices[next_ix_b] == False:
            edge_loop[edge_loop_ix] = e.b
            edge_loop_ix += 1
            ix = next_ix_b
        else:
            break

    return edge_loop_ix

def next_unvisited(visited_edge_indices: list[bool],
                   edge_indices_length: int):
    for i in range(edge_indices_length):
        if visited_edge_indices[i] == False:
            return i
    return -1

def edge_loop(edge_polygons: list[EdgePolygon],
              edge_indices: list[int],
              edge_indices_length: int,
              graph: list[Graph],
              edge_loops: list[int],
              edge_loop_lengths: list[int],
              max_edge_loops: int):
    visited_edge_indices = [False] * edge_indices_length

    edge_loop_ix = 0
    i = 0
    #while i < max_edge_loops:
    while True:
        start = next_unvisited(visited_edge_indices, edge_indices_length)
        if start == -1:
            break
        assert i < max_edge_loops

        new_edge_loop_ix = edge_loop_inner(edge_polygons,
                                           edge_indices,
                                           graph,
                                           visited_edge_indices,
                                           start,
                                           edge_loops,
                                           edge_loop_ix)
        length = new_edge_loop_ix - edge_loop_ix
        edge_loop_lengths[i] = length
        edge_loop_ix = new_edge_loop_ix

        i += 1

    return i

def list_init(l, init):
    for i in range(len(l)):
        l[i] = init(0, 0)

def print_edge_loops(edge_loops: list[int],
                     edge_loop_lengths: list[int],
                     edge_loop_count: int):
    edge_loop_ix = 0
    for i in range(edge_loop_count):
        length = edge_loop_lengths[i]
        l = [edge_loops[edge_loop_ix + j] for j in range(length)]
        s = ", ".join(map(str, l))
        print(f"loop {i} [{s}]")
        edge_loop_ix += length

def reset_mesh_colors(mesh):
    for color in mesh.color_attributes['Attribute'].data:
        color.color = (1, 1, 1, 1)

def color_edge_loop(mesh,
                    edge_loops: list[int],
                    edge_loop_lengths: int,
                    edge_loop_count: int):
    edge_loop_ix = 0
    for i in range(edge_loop_count):
        length = edge_loop_lengths[i]

        for j in range(length):
            fraction = j / length
            r, g, b = colorsys.hsv_to_rgb(fraction, 1, j != 0)

            vertex_index = edge_loops[edge_loop_ix + j]
            for polygon in mesh.polygons:
                if vertex_index in polygon.vertices:
                    ix = list(polygon.vertices).index(vertex_index)
                    loop_ix = polygon.loop_indices[ix]
                    mesh.color_attributes['Attribute'].data[loop_ix].color_srgb = (r, g, b, 1)

        edge_loop_ix += length

def cast_ray(light: Vector,
             start: Vector):
    ray = start - light
    ray.normalize()
    return start + (ray * 7.0)

def link_bmesh(bm, name):
    mesh = bpy.data.meshes.new(name)
    bm.to_mesh(mesh)
    bm.free()
    object = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(object)

def shadow_volume_mesh_rays(bm,
                            bm_position: list[bmesh.types.BMVert],
                            bm_cast_position: list[bmesh.types.BMVert],
                            edge_loops: list[int],
                            edge_loop_ix: int,
                            edge_loop_length: int):
    for i in range(edge_loop_length):
        j = (i + 1) % edge_loop_length

        i1 = edge_loops[edge_loop_ix + i]
        i2 = edge_loops[edge_loop_ix + j]

        a = bm_position[i1]
        b = bm_position[i2]
        c = bm_cast_position[i2]
        d = bm_cast_position[i1]

        bm.faces.new([a, b, c, d])

def shadow_volume_end_caps(bm,
                           bm_position: list[bmesh.types.BMVert],
                           bm_cast_position: list[bmesh.types.BMVert],
                           mesh,
                           indicators: list[float]):
    for i in range(len(mesh.polygons)):
        p = mesh.polygons[i]
        if indicators[i] > 0:
            a = bm_position[p.vertices[0]]
            b = bm_position[p.vertices[1]]
            c = bm_position[p.vertices[2]]
            d = bm_position[p.vertices[3]]
            bm.faces.new([a, b, c, d])
        else:
            a = bm_cast_position[p.vertices[0]]
            b = bm_cast_position[p.vertices[1]]
            c = bm_cast_position[p.vertices[2]]
            d = bm_cast_position[p.vertices[3]]
            bm.faces.new([a, b, c, d])

def main():
    light = bpy.data.objects['Light'].location
    torus = bpy.data.objects['Torus']

    position = [0] * len(torus.data.vertices)
    for i, vertex in enumerate(torus.data.vertices):
        position[i] = torus.matrix_world @ vertex.co

    polygon_normal = [0] * len(torus.data.polygon_normals)
    for i, normal in enumerate(torus.data.polygon_normals):
        n = torus.matrix_world.to_3x3() @ normal.vector
        n.normalize()
        polygon_normal[i] = n

    mesh = torus.data

    indicators = [0] * len(torus.data.polygon_normals)

    face_indicators(light,
                    position,
                    polygon_normal,
                    mesh,
                    # outputs
                    indicators)

    # select_light_faces(indicators)

    edge_polygons = build_edge_polygons(mesh)

    edge_indices = [0] * len(edge_polygons)

    edge_indices_length = object_silhouette(indicators,
                                            edge_polygons,
                                            # outputs
                                            edge_indices)

    # select_silhouette(edge_polygons, edge_indices, edge_indices_length)

    # graph is a mapping from vertex indices to edge_indices
    graph = [0] * len(mesh.vertices)
    list_init(graph, Graph)
    edge_loop_graph(mesh,
                    edge_polygons,
                    edge_indices,
                    edge_indices_length,
                    # outputs
                    graph)

    #print(graph)

    max_edge_loops = 2
    edge_loops = [0] * edge_indices_length
    edge_loop_lengths = [0] * max_edge_loops
    edge_loop_count = edge_loop(edge_polygons,
                                edge_indices,
                                edge_indices_length,
                                graph,
                                edge_loops,
                                edge_loop_lengths,
                                max_edge_loops)
    print("edge loop count", edge_loop_count)

    print_edge_loops(edge_loops, edge_loop_lengths, edge_loop_count)

    """
    reset_mesh_colors(mesh)
    color_edge_loop(mesh,
                    edge_loops,
                    edge_loop_lengths,
                    edge_loop_count)
    """

    cast_position = [0] * len(mesh.vertices)
    for i in range(len(mesh.vertices)):
        cast_position[i] = cast_ray(light, position[i])

    bm = bmesh.new()
    bm_position = list(map(bm.verts.new, position))
    bm_cast_position = list(map(bm.verts.new, cast_position))

    edge_loop_ix = 0
    for i in range(edge_loop_count):
        edge_loop_length = edge_loop_lengths[i]
        shadow_volume_mesh_rays(bm,
                                bm_position,
                                bm_cast_position,
                                edge_loops,
                                edge_loop_ix,
                                edge_loop_length)
        edge_loop_ix += edge_loop_length

    shadow_volume_end_caps(bm,
                           bm_position,
                           bm_cast_position,
                           mesh,
                           indicators)

    link_bmesh(bm, "test")

angle = 50
def rotate():
    global angle

    torus = bpy.data.objects['Torus']
    torus.rotation_mode = 'XYZ'
    torus.rotation_euler = Euler((0, angle / 100, 0), 'XYZ')

    try:
        main()
    except:
        import traceback
        for line in traceback.format_exc().split('\n'):
            print(line)
        return

    angle += 1
    print(angle)
    if angle < 1000:
        bpy.app.timers.register(rotate, first_interval=0.01)

#rotate()
main()
