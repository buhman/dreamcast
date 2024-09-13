import random
import math
from PIL import Image
import sys
from itertools import chain
from pprint import pprint
from itertools import starmap
import struct

def random_k_points(k, points):
    points = list(points)
    out = []
    while len(out) < k and len(points) != 0:
        ix = random.randint(0, len(points) - 1)
        if points[ix] not in out:
            value = points.pop(ix)
            out.append(value)
    assert len(out) == k
    return out

def distance_one(point, centroid):
    x0, y0, z0 = point
    x1, y1, z1 = centroid
    xd = (x1 - x0) ** 2
    yd = (y1 - y0) ** 2
    zd = (z1 - z0) ** 2
    return xd + yd + zd

def distance(point, centroid):
    assert len(point) == 4
    assert len(centroid) == 4
    return math.sqrt(sum(starmap(distance_one, zip(point, centroid))))

def argmin(l):
    min = (0, l[0])
    for ix, v in enumerate(l[1:]):
        if v < min[1]:
            min = (ix+1, v)
    return min[0]

def calculate_centroid_one(points):
    xs = 0
    ys = 0
    zs = 0
    for x, y, z in points:
        xs += x
        ys += y
        zs += z
    return (xs / len(points), ys / len(points), zs / len(points))

def calculate_centroid(points):
    t = tuple(map(calculate_centroid_one, zip(*points)))
    assert len(t) == 4, t
    return t

def k_means_cluster(k, points):
    # Initialization: choose k centroids (Forgy, Random Partition, etc.)
    centroids = random_k_points(k, points)
    assert len(centroids) == k

    # Initialize clusters list
    clusters = [[] for _ in range(k)]

    # Loop until convergence
    converged = False
    while not converged:
        # Clear previous clusters
        clusters = [[] for _ in range(k)]

        # Assign each point to the "closest" centroid
        for point in points:
            distances_to_each_centroid = [distance(point, centroid) for centroid in centroids]
            cluster_assignment = argmin(distances_to_each_centroid)
            clusters[cluster_assignment].append(point)
        assert(all(len(c) != 0 for c in clusters))
        print(clusters)
        # Calculate new centroids
        #   (the standard implementation uses the mean of all points in a
        #     cluster to determine the new centroid)
        new_centroids = [calculate_centroid(cluster) for cluster in clusters]

        converged = (new_centroids == centroids)
        centroids = new_centroids

        if converged:
            return clusters

def palettize(centroids, point):
    distances_to_each_centroid = [distance(point, centroid) for centroid in centroids]
    cluster_assignment = argmin(distances_to_each_centroid)
    return cluster_assignment

def rgb565(color):
    r, g, b = color
    return r >> 3, g >> 2, b >> 3

def pixels_to_codebook(pixels, width, height):
    for ty in range(height // 2):
        for tx in range(width // 2):
            ai = ((ty * 2) + 0) * width + ((tx * 2) + 0)
            bi = ((ty * 2) + 1) * width + ((tx * 2) + 0)
            ci = ((ty * 2) + 0) * width + ((tx * 2) + 1)
            di = ((ty * 2) + 1) * width + ((tx * 2) + 1)
            codeword = pixels[ai], pixels[bi], pixels[ci], pixels[di]
            yield codeword

def codebook_codepixels_to_pixels(codebook, codepixels, width, height):
    canvas = [0] * (width * height)
    for ty in range(height // 2):
        for tx in range(width // 2):
            codepixel = codepixels[ty * width // 2 + tx]
            assignment = palettize(codebook, codepixel)
            ap, bp, cp, dp = codebook[assignment]
            ai = ((ty * 2) + 0) * width + ((tx * 2) + 0)
            bi = ((ty * 2) + 1) * width + ((tx * 2) + 0)
            ci = ((ty * 2) + 0) * width + ((tx * 2) + 1)
            di = ((ty * 2) + 1) * width + ((tx * 2) + 1)
            canvas[ai] = ap
            canvas[bi] = bp
            canvas[ci] = cp
            canvas[di] = dp
    return canvas

def remove_gamma(c):
    c /= 255
    if c <= 0.04045:
        return c / 12.92
    else:
        return ((c + 0.055)/1.055) ** 2.4

def apply_gamma(c):
    if c <= 0.0031308:
        c2 = c * 12.92
    else:
        c2 = 1.055 * (c ** (1/2.4)) - 0.055
    return round(c2 * 255)

def apply_gamma_v(v):
    assert len(v) == 3
    return tuple(map(apply_gamma, v))

def remove_gamma_v(v):
    assert len(v) == 3
    return tuple(map(remove_gamma, v))

for i in range(0, 256):
    rt = apply_gamma(remove_gamma(i))
    assert rt == i, (rt, i)

def mat3x3_mul_v(mat, v):
    def dot(row):
        return mat[row][0] * v[0] + \
               mat[row][1] * v[1] + \
               mat[row][2] * v[2]

    return tuple((dot(0), dot(1), dot(2)))

def srgb_to_ciexyz(color):
    mat = [[0.4124564, 0.3575761, 0.1804375],
           [0.2126729, 0.7151522, 0.0721750],
           [0.0193339, 0.1191920, 0.9503041]]
    return mat3x3_mul_v(mat, color)

def ciexyz_to_srgb(color):
    mat = [[ 3.2404542, -1.5371385, -0.4985314],
           [-0.9692660,  1.8760108,  0.0415560],
           [ 0.0556434, -0.2040259,  1.0572252]]
    return mat3x3_mul_v(mat, color)

def rgb24(color):
    r, g, b = color
    return round(r) * (1 << 3), round(g) * (1 << 2), round(b) * (1 << 3)

for _ in range(256):
    rcolor = tuple(random.randint(0, 255) for _ in range(3))
    rtcolor = ciexyz_to_srgb(srgb_to_ciexyz(rcolor))
    assert rcolor == tuple(map(round, rtcolor))

def write_binary_vq(f, codebook, codepixels):
    # ᴎ
    for colors in codebook:
        for color in colors:
            r, g, b = map(round, color)
            assert r <= 31 and g <= 63 and b <= 31
            n = (r << 11) | (g << 5) | (b << 0)
            f.write(struct.pack('<H', n))
    for codepixel in codepixels:
        assignment = palettize(codebook, codepixel)
        assert assignment <= 255 and assignment >= 0
        f.write(bytes([assignment]))

def do(filename, output):
    with Image.open(filename) as im:
        pixels = list(im.convert("RGB").getdata())
        #ciexyz_pixels = [srgb_to_ciexyz(p) for p in pixels]
        rgb565_pixels = [rgb565(p) for p in pixels]
        width, height = im.size
        codepixels = list(pixels_to_codebook(rgb565_pixels, width, height))

        clusters = k_means_cluster(256, codepixels)

        codebook = list(calculate_centroid(cluster) for cluster in clusters)

        canvas_ciexyz = codebook_codepixels_to_pixels(codebook, codepixels, width, height)
        canvas_rgb = [tuple(map(round, rgb24(p))) for p in canvas_ciexyz]

        palimage = Image.new('RGB', im.size)
        palimage.putdata(canvas_rgb)
        palimage.save(output)

        with open(output.split('.', maxsplit=1)[0] + '.vq.bin', 'wb') as f:
            write_binary_vq(f, codebook, codepixels)

if __name__ == "__main__":
    in_file = sys.argv[1]
    out_file = sys.argv[2]
    do(in_file, out_file)
