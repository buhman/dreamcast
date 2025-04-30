import sys
from os import path as os_path
from generate import renderer

with open(sys.argv[1], "r") as f:
    lines = f.read().strip().split('\n')

prefix = "models/players"

texture_ix = 0
textures = {}

def get_texture_ix(path):
    global texture_ix
    global textures
    if path not in textures:
        textures[path] = texture_ix
        texture_ix += 1
    return textures[path]

with open(sys.argv[2], "w") as f:
    for line in lines:
        name, path = line.split(",")
        path = path.removeprefix(prefix)
        path = path.removesuffix(".tga")
        path = "model" + path + ".jpg"
        if os_path.exists(path):
            ix = get_texture_ix(path)
            f.write(f"   {ix}, // {line}\n")
        else:
            f.write(f"  -1, // {line}\n")

from texture_gen import render_texture_metadatas

textures = [k for k, v in sorted(textures.items(), key=lambda kv: kv[1])]

mipmapped = False
prefix = ""
suffix = "_vq"
render, out = renderer()
bytes_per_pixel = 0.25
render(render_texture_metadatas(textures, mipmapped, bytes_per_pixel, prefix, suffix))
with open(sys.argv[3], "w") as f:
    f.write(out.getvalue())
