from typing import Any, Union
from dataclasses import dataclass

# untagged objects

@dataclass
class ColorRGBA:
    r: float
    g: float
    b: float
    a: float

@dataclass
class ColorRGB:
    r: float
    g: float
    b: float

@dataclass
class Matrix4x4:
    v: list[float]

@dataclass
class Vector:
    x: float
    y: float
    z: float

@dataclass
class MeshFace:
    nFaceVertexIndices: int
    faceVertexIndices: list[int]

@dataclass
class Coords2D:
    u: float
    v: float

@dataclass
class Reference:
    name: bytes

@dataclass
class FloatKeys:
    nValues: int
    values: list[float]

@dataclass
class TimedFloatKeys:
    time: int
    tfkeys: FloatKeys

# tagged objects

@dataclass
class Header:
    major: int
    minor: int
    flags: int

@dataclass
class Material:
    faceColor: ColorRGBA
    power: float
    specularColor: ColorRGB
    emissiveColor: ColorRGB

    objects: list[Any]

@dataclass
class TextureFilename:
    filename: str

@dataclass
class Frame:
    objects: list[Any]

@dataclass
class FrameTransformMatrix:
    frameMatrix: Matrix4x4

@dataclass
class Mesh:
    nVertices: int
    vertices: list[Vector]
    nFaces: int
    faces: list[MeshFace]
    objects: list[Any]

@dataclass
class MeshMaterialList:
    nMaterials: int
    nFaceIndices: int
    faceIndices: list[int]
    objects: list[Material]

@dataclass
class MeshNormals:
    nNormals: int
    normals: list[Vector]
    nFaceNormals: int
    faceNormals: list[MeshFace]

@dataclass
class MeshTextureCoords:
    nTextureCoords: int
    textureCoords: list[Coords2D]

@dataclass
class AnimationKey:
    keyType: int
    nKeys: int
    keys: list[TimedFloatKeys]

@dataclass
class AnimationOptions:
    openClosed: int
    positionQuality: int

@dataclass
class Animation:
    objects: list[Union[Reference, AnimationKey, AnimationOptions]]

@dataclass
class AnimationSet:
    objects: list[Animation]
