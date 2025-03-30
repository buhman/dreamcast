import random
import math
from PIL import Image
import sys
from itertools import chain
from pprint import pprint

def random_k_points(k, points):
    points = list(points)
    out = []
    while len(out) < k and len(points) != 0:
        ix = random.randint(0, len(points) - 1)
        if points[ix] not in out:
            value = points.pop(ix)
            out.append(value)
    return out

def distance(point, centroid):
    x0, y0, z0 = point
    x1, y1, z1 = centroid
    try:
        xd = (x1 - x0) ** 2
    except OverflowError:
        print(point, x1, x0)
    yd = (y1 - y0) ** 2
    zd = (z1 - z0) ** 2
    return math.sqrt(xd + yd + zd)

def argmin(l):
    min = (0, l[0])
    for ix, v in enumerate(l[1:]):
        if v < min[1]:
            min = (ix+1, v)
    return min[0]

def calculate_centroid(points):
    xs = 0
    ys = 0
    zs = 0
    for x, y, z in points:
        xs += x
        ys += y
        zs += z
    return (xs / len(points), ys / len(points), zs / len(points))

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

        # Calculate new centroids
        #   (the standard implementation uses the mean of all points in a
        #     cluster to determine the new centroid)
        #print(clusters)
        new_centroids = [calculate_centroid(cluster) for cluster in clusters]

        converged = (new_centroids == centroids)
        centroids = new_centroids

        if converged:
            return clusters

def palettize(centroids, point):
    distances_to_each_centroid = [distance(point, centroid) for centroid in centroids]
    cluster_assignment = argmin(distances_to_each_centroid)
    return cluster_assignment

def pixel_sub(a, b):
    return (
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    )

def pixel_mul(a, s):
    return (
        a[0] * s,
        a[1] * s,
        a[2] * s
    )

def pixel_add(a, b):
    return (
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2]
    )

def dither(width, height, palette, pixels):
    pixels = list(pixels)

    def ix(x, y):
        return y * width + x

    def add_error(x, y, error, scale):
        if x >= width or y >= height:
            return
        old_value = pixels[ix(x, y)]
        new_value = pixel_add(old_value, pixel_mul(error, scale))
        pixels[ix(x, y)] = new_value

    for y in range(height):
        for x in range(width):
            old_pixel = pixels[y * width + x]
            new_pixel_ix = palettize(palette, old_pixel)
            new_pixel = palette[new_pixel_ix]

            pixels[ix(x, y)] = new_pixel_ix

            quant_error = pixel_sub(old_pixel, new_pixel)

            add_error(x + 1, y + 0, quant_error, 7 / 16)
            add_error(x - 1, y + 1, quant_error, 3 / 16)
            add_error(x + 0, y + 1, quant_error, 5 / 16)
            add_error(x + 1, y + 1, quant_error, 1 / 16)

    return pixels

output = sys.argv[2]
with Image.open(sys.argv[1]) as im:
    pixels = list(im.convert("RGB").getdata())
    clusters = k_means_cluster(4, pixels)
    palette = list(calculate_centroid(cluster) for cluster in clusters)
    #print(palette)
    palimage = Image.new('P', im.size)
    palimage.putpalette(map(int, list(chain.from_iterable(palette))))
    palimage.putdata([palettize(palette, pixel) for pixel in pixels])
    palimage.save(sys.argv[2])

    ditherimage = Image.new('P', im.size)
    ditherimage.putpalette(map(int, list(chain.from_iterable(palette))))
    ditherimage.putdata(dither(*im.size, palette, pixels))
    ditherimage.save(sys.argv[3])
