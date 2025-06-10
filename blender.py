import bpy
from os import path
from collections import defaultdict

def sprint(*text):
    screen = bpy.data.screens['Scripting']
    for area in screen.areas:
        if area.type != "CONSOLE":
            continue
        override = {'screen': screen, 'area': area}
        with bpy.context.temp_override(**override):
            bpy.ops.console.scrollback_append(text=" ".join(map(str, text)))

print = sprint

def render_vec3(v):
    return f"{{{v.x:.6f}, {v.y:.6f}, {v.z:.6f}}}"

def render_vec2(v):
    return f"{{{v.x:.6f}, {v.y:.6f}}}"

def render_mesh_vertices(f, name, vertices):
    f.write(f"const vec3 {name}_position[] = {{\n")
    for vertex in vertices:
        f.write(f"  {render_vec3(vertex.co)},\n")
    f.write("};\n\n")

def render_vertex_normals(f, name, vertices):
    f.write(f"const vec3 {name}_normal[] = {{\n")
    for vertex in vertices:
        f.write(f"  {render_vec3(vertex.normal)},\n")
    f.write("};\n\n")

def render_polygon_normals(f, name, polygon_normals):
    f.write(f"const vec3 {name}_polygon_normal[] = {{\n")
    for normal in polygon_normals:
        f.write(f"  {render_vec3(normal.vector)},\n")
    f.write("};\n\n")

def sort_by_material(polygons):
    return sorted(polygons, key=lambda p: p.material_index)

def render_polygons(f, name, polygons):
    f.write(f"const polygon {name}_polygons[] = {{\n")

    uv_ix = 0
    for i, polygon in enumerate(polygons):
        indices = [*polygon.vertices, polygon.material_index, uv_ix]
        uv_ix += len(polygon.vertices)
        s = ", ".join(map(str, indices))
        if len(polygon.vertices) == 4:
            f.write(f"  {{{s}}},\n")
        else:
            f.write(f"  {{0, 0, 0, 0, -1}}, // {{{s}}}\n")
    f.write("};\n\n")

def render_polygon_edge_pairs(f, name, polygons):
    by_edge = defaultdict(list)
    for i, polygon in enumerate(polygons):
        for edge in polygon.edge_keys:
            by_edge[frozenset(edge)].append(i)

    f.write(f"const edge_polygon {name}_edge_polygons[] = {{\n")
    if all(len(p) == 2 for p in by_edge.values()):
        for edge, polygons in by_edge.items():
            edges = sorted(list(edge))
            assert len(edges) == 2, edges
            assert len(polygons) == 2, polygons
            f.write(f"  {{{{{edges[0]}, {edges[1]}}}, {{{polygons[0]}, {polygons[1]}}}}},\n")
    else:
        f.write("// non-solid polygon\n")
    f.write("};\n\n")

def render_uv_map(f, name, name2, uvm):
    f.write(f"const vec2 {name}_{name2}_uvmap[] = {{\n")
    for uv in uvm:
        s = render_vec2(uv.vector)
        f.write(f"  {s},\n")
    f.write("};\n\n")

def render_location(f, location):
    s = render_vec3(location)
    f.write(f"  .location = {s},\n")

def render_scale(f, scale):
    s = render_vec3(scale)
    f.write(f"  .scale = {s},\n")

def render_rotation_axis_angle(f, r):
    r = f"{{{r[1]:.6f}, {r[2]:.6f}, {r[3]:.6f}, {r[0]:.6f}}}"
    f.write(f"  .rotation = {r}, // rotation_axis_angle (XYZ T)\n")

def render_rotation_quaternion(f, r):
    r = f"{{{r[1]:.6f}, {r[2]:.6f}, {r[3]:.6f}, {r[0]:.6f}}}"
    f.write(f"  .rotation = {r}, // quaternion (XYZW)\n")

def render_mesh(f, name, mesh):
    f.write(f"const vec2 * {name}_uv_layers[] = {{\n")
    for layer_name in mesh.uv_layers.keys():
        f.write(f"  {name}_{translate_name(layer_name)}_uvmap,\n");
    f.write( "};\n\n")

    f.write(f"const mesh {name} = {{\n")
    f.write(f"  .position = {name}_position,\n")
    f.write(f"  .position_length = (sizeof ({name}_position)) / (sizeof ({name}_position[0])),\n")
    f.write(f"  .normal = {name}_normal,\n")
    f.write(f"  .normal_length = (sizeof ({name}_normal)) / (sizeof ({name}_normal[0])),\n")
    f.write(f"  .polygon_normal = {name}_polygon_normal,\n")
    f.write(f"  .polygon_normal_length = (sizeof ({name}_polygon_normal)) / (sizeof ({name}_polygon_normal[0])),\n")
    f.write(f"  .polygons = {name}_polygons,\n")
    f.write(f"  .polygons_length = (sizeof ({name}_polygons)) / (sizeof ({name}_polygons[0])),\n")
    f.write(f"  .uv_layers = {name}_uv_layers,\n")
    f.write(f"  .uv_layers_length = (sizeof ({name}_uv_layers)) / (sizeof ({name}_uv_layers[0])),\n")
    f.write(f"  .materials = {name}_materials,\n")
    f.write(f"  .materials_length = (sizeof ({name}_materials)) / (sizeof ({name}_materials[0])),\n")
    f.write(f"  .edge_polygons = {name}_edge_polygons,\n");
    f.write(f"  .edge_polygons_length = (sizeof ({name}_edge_polygons)) / (sizeof ({name}_edge_polygons[0])),\n")
    f.write( "};\n\n")

def translate_name(name):
    return name.replace(".", "_").replace("-", "_").replace(" ", "_")

def mesh_objects(collections):
    objects = set()
    for collection in collections:
        if collection.hide_render:
            continue
        for object in collection.objects:
            assert object.name not in objects, object.name
            objects.add(object.name)
            if object.hide_render:
                continue
            if object.type == "MESH":
                yield object

def mesh_meshes(collections):
    mesh_names = set()
    for object in mesh_objects(collections):
        mesh = object.data
        if mesh.name in mesh_names:
            continue
        mesh_names.add(mesh.name)
        yield mesh

def get_texture(material):
    assert material.use_nodes, material.name
    for node in material.node_tree.nodes:
        if node.type == "TEX_IMAGE":
            return node.image

_offset = 0
texture_offsets = {}
prefix = "model_cars_"

def get_texture_offset(image):
    global _offset
    if image.name in texture_offsets:
        value = texture_offsets[image.name]
        return value
    value = _offset
    texture_offsets[image.name] = value
    width, height = image.size
    _offset += width * height * 2
    return value

def texture_data_name(name):
    name = path.splitext(name)[0]
    name = translate_name(name)
    return f"{prefix}{name}_data"

def render_mesh_materials(f, name, materials):
    f.write(f"const mesh_material {name}_materials[] = {{\n")
    for material in materials:
        image = get_texture(material)
        if image is not None:
            f.write(f"  {{ // {material.name} {image.name}\n")
            width, height = image.size
            offset = get_texture_offset(image)
            f.write(f"    .width = {width},\n")
            f.write(f"    .height = {height},\n")
            f.write(f"    .offset = {offset},\n")
            f.write("  },\n")
        else:
            f.write("  {},\n")
    f.write("};\n")

def render_materials(f):
    f.write("const material materials[] = {\n")
    for image_name, offset in texture_offsets.items():
        name = texture_data_name(image_name)
        f.write("  {\n")
        f.write(f"    .start = (void *)&_binary_{name}_start,\n")
        f.write(f"    .size = (int)&_binary_{name}_size,\n")
        f.write(f"    .offset = {offset},\n")
        f.write("  },\n")
    f.write("};\n\n");

def export_meshes(f):
    for mesh in mesh_meshes(bpy.data.collections):
        #mesh.vertex_normals
        #mesh.vertex_colors
        #mesh.vertices
        #mesh.uv_layers
        #mesh.polygons
        #mesh.polygon_normals
        #mesh.name

        mesh_name = "mesh_" + translate_name(mesh.name)

        render_mesh_vertices(f, mesh_name, mesh.vertices)
        for layer_name, layer in mesh.uv_layers.items():
            render_uv_map(f, mesh_name, translate_name(layer_name), layer.uv)
        render_vertex_normals(f, mesh_name, mesh.vertices)
        render_polygon_normals(f, mesh_name, mesh.polygon_normals)
        render_polygons(f, mesh_name, mesh.polygons)
        render_polygon_edge_pairs(f, mesh_name, mesh.polygons)
        render_mesh_materials(f, mesh_name, mesh.materials)

        render_mesh(f, mesh_name, mesh);

        #mesh.polygons[0].vertices
        # [0, 1, 3, 2]

        # v = mesh.vertices[0]
        # v.normal
        # v.index

def mesh_objects_sorted(objects):
    def key(o):
        return (o.data.name, o.name)
    return sorted(mesh_objects(objects), key=key)

def export_objects(f):
    f.write("const object objects[] = {\n")
    for object in mesh_objects_sorted(bpy.data.collections):

        #object.rotation_mode = 'AXIS_ANGLE'
        #object.name
        #object.rotation_axis_angle
        #object.rotation_euler
        #object.location

        obj_name = "object_" + translate_name(object.name)

        f.write(f"  {{ // {obj_name}\n")

        obj_mesh_name = "mesh_" + translate_name(object.to_mesh().name)

        f.write("  ")
        f.write(f"  .mesh = &{obj_mesh_name},\n")

        location, rotation, scale = object.matrix_world.decompose()
        f.write("  ")
        render_scale(f, scale)
        f.write("  ")
        render_rotation_quaternion(f, rotation)
        f.write("  ")
        render_location(f, location)

        f.write("  },\n")
    f.write("};\n\n")

def export_scene(f):
    export_meshes(f)
    export_objects(f)
    render_materials(f)

home = path.expanduser('~')
with open(path.join(home, "output.h"), "w") as f:
    offset = 0
    export_scene(f)
