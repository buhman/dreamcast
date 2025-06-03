import bpy
import bmesh
from mathutils import Vector, Matrix
import copy

from math import sin, cos, pi

def knot(t):
    x = sin(t) + 2 * sin(2 * t)
    y = cos(t) - 2 * cos(2 * t)
    z = -sin(3 * t)

    return Vector((x, y, z))

def rr(v: Vector,
       k: Vector, # axis
       t: float):
    return v * cos(t) + k.cross(v) * sin(t) + k * (k.dot(v)) * (1 - cos(t))

def radial_segments(bm, a, n0, n, radial_surface_n, segments):
    for i in range(segments):
        t = (i / segments) * 2 * pi

        rn = rr(n, n0, t)
        rn.normalize()
        surface = bm.verts.new(a.co + rn * 0.6)
        radial_surface_n[i] = surface

def radial_faces(bm, radial_surface_p, radial_surface_n, segments):
    for i in range(segments):
        j = (i + 1) % segments

        bm.faces.new([
            radial_surface_n[i],
            radial_surface_n[j],
            radial_surface_p[j],
            radial_surface_p[i]
        ])

def knot_edges(points, segments):
    bm = bmesh.new()

    knot_centers = []

    for i in range(points):
        t = (i / points) * 2 * pi
        center = bm.verts.new(knot(t))
        knot_centers.append(center)

    radial_surface_p = [0] * segments
    radial_surface_n = [0] * segments
    radial_surface_f = None

    for i in range(points):
        ip = (i + 1) % points
        im = (i - 1) % points
        a = knot_centers[i]
        b = knot_centers[ip]
        c = knot_centers[im]

        n0 = ((b.co - a.co) + (a.co - c.co)) / 2
        n0.normalize()
        n1 = Vector(a.co)
        n = n0.cross(-n1)
        n.normalize()

        radial_segments(bm, a, n0, n,
                        radial_surface_n,
                        segments)

        if i == 0:
            radial_surface_f = copy.copy(radial_surface_n)
        else:
            radial_faces(bm, radial_surface_p, radial_surface_n, segments)

        tmp = radial_surface_p
        radial_surface_p = radial_surface_n
        radial_surface_n = tmp

    radial_faces(bm, radial_surface_f, radial_surface_p, segments)

    mesh = bpy.data.meshes.new("test")
    bm.to_mesh(mesh)
    bm.free()
    object = bpy.data.objects.new("test", mesh)
    bpy.context.scene.collection.objects.link(object)

def delete_test_objects(collection):
    for o in collection.objects:
        if o.name.startswith("test"):
            collection.objects.unlink(o)

delete_test_objects(bpy.context.scene.collection)
knot_edges(256, 32)
