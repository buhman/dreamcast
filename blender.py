import bpy

def render_vec3(v):
    return f"{{{v.x}, {v.y}, {v.z}}}"

def render_vec2(v):
    return f"{{{v.x}, {v.y}}}"

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

def render_polygons(f, name, polygons):
    f.write(f"const polygon {name}_polygons[] = {{\n")
    for i, polygon in enumerate(polygons):
        s = ", ".join(map(str, polygon.vertices))
        c = "// " if len(polygon.vertices) != 4 else ""

        f.write(f"  {c}{{{s}}},\n")
    f.write("};\n\n")

def render_uv_map(f, name, name2, uvm):
    f.write(f"const vec2 {name}_{name2}_uvmap[] = {{\n")
    print(uvm)
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
    r = f"{{{r[0]}, {r[1]}, {r[2]}, {r[3]}}}"
    f.write(f"  .rotation = {r},\n")

def render_mesh(f, name, mesh):
    f.write(f"const vec2 * {name}_uv_layers[] = {{\n")
    for layer_name in mesh.uv_layers.keys():
        f.write(f"  {name}_{layer_name}_uvmap,\n");
    f.write( "};\n\n")

    f.write(f"const mesh {name} = {{\n")
    f.write(f"  .position = {name}_position,\n")
    f.write(f"  .position_length = (sizeof ({name}_position)) / (sizeof ({name}_position[0])),\n")
    f.write(f"  .polygon_normal = {name}_polyon_normal,\n")
    f.write(f"  .polygon_normal_length = (sizeof ({name}_polygon_normal)) / (sizeof ({name}_polygon_normal[0])),\n")
    f.write(f"  .polygons = {name}_polygons,\n")
    f.write(f"  .polygons_length = (sizeof ({name}_polygons)) / (sizeof ({name}_polygons[0])),\n")
    f.write(f"  .uv_layers = {name}_uv_layers,\n")
    f.write(f"  .uv_layers_length = (sizeof ({name}_uv_layers)) / (sizeof ({name}_uv_layers[0])),\n")
    f.write( "};\n\n")

def translate_name(name):
    return name.replace(".", "_").replace("-", "_")

def export_scene(f):
    meshes = set()

    for object in bpy.context.scene.objects:
        #mesh = c.to_mesh()
        #mesh.vertex_normals
        #mesh.vertex_colors
        #mesh.vertices
        #mesh.uv_layers
        #mesh.polygons
        #mesh.polygon_normals
        #mesh.name

        mesh = object.to_mesh()
        if mesh.name in meshes:
            continue
        meshes.add(mesh.name)

        mesh_name = "mesh_" + translate_name(mesh.name)

        render_mesh_vertices(f, mesh_name, mesh.vertices)
        for layer_name, layer in mesh.uv_layers.items():
            render_uv_map(f, mesh_name, layer_name, layer.uv)
        #render_vertex_normals(f, mesh_name, mesh.vertices)
        render_polygon_normals(f, mesh_name, mesh.polygon_normals)
        render_polygons(f, mesh_name, mesh.polygons)


        render_mesh(f, mesh_name, mesh);

        #mesh.polygons[0].vertices
        # [0, 1, 3, 2]

        # v = mesh.vertices[0]
        # v.normal
        # v.index

    f.write("const struct object objects[] = {\n")
    for object in bpy.context.scene.objects:
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

        f.write("  ")
        render_scale(f, object.scale)
        f.write("  ")
        render_rotation_axis_angle(f, object.rotation_axis_angle)
        f.write("  ")
        render_location(f, object.location)

        f.write("  },\n")
    f.write("};\n\n")

with open("output.h", "w") as f:
    export_scene(f)
