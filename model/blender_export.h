struct polygon {
  int a, b, c, d;
};

struct mesh {
  const vec3 * position;
  const int position_length;
  const vec3 * normal;
  const int normal_length;
  const polygon * polygons;
  const int polygons_length;
  const vec2 ** uv_layers;
  const int uv_layers_length;
};

struct object {
  const struct mesh * mesh;
  const vec3 scale;
  const vec4 rotation;
  const vec3 location;
};
