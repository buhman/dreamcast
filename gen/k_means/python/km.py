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
    xd = (x1 - x0) ** 2
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
        print(clusters)
        new_centroids = [calculate_centroid(cluster) for cluster in clusters]

        converged = (new_centroids == centroids)
        centroids = new_centroids

        if converged:
            return clusters

def palettize(centroids, point):
    distances_to_each_centroid = [distance(point, centroid) for centroid in centroids]
    cluster_assignment = argmin(distances_to_each_centroid)
    return cluster_assignment

output = sys.argv[2]
with Image.open(sys.argv[1]) as im:
    pixels = list(im.convert("RGB").getdata())
    clusters = k_means_cluster(16, pixels)
    palette = list(calculate_centroid(cluster) for cluster in clusters)
    print(palette)
    palimage = Image.new('P', im.size)
    palimage.putpalette(map(int, list(chain.from_iterable(palette))))
    palimage.putdata([palettize(palette, pixel) for pixel in pixels])
    palimage.save(sys.argv[2])
