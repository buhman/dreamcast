const vec3 mesh_Plane_position[] = {
  {-1.000000, -1.000000, 0.000000},
  {1.000000, -1.000000, 0.000000},
  {-1.000000, 1.000000, 0.000000},
  {1.000000, 1.000000, 0.000000},
};

const vec2 mesh_Plane_UVMap_uvmap[] = {
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
};

const vec2 mesh_Plane_lightmap_uvmap[] = {
  {0.997996, 0.002004},
  {0.002004, 0.002004},
  {0.002004, 0.997996},
  {0.997996, 0.997996},
};

const vec3 mesh_Plane_normal[] = {
  {0.000000, 0.000000, 1.000000},
  {0.000000, 0.000000, 1.000000},
  {0.000000, 0.000000, 1.000000},
  {0.000000, 0.000000, 1.000000},
};

const vec3 mesh_Plane_polygon_normal[] = {
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_Plane_polygons[] = {
  {0, 1, 3, 2},
};

const vec2 * mesh_Plane_uv_layers[] = {
  mesh_Plane_UVMap_uvmap,
  mesh_Plane_lightmap_uvmap,
};

const mesh mesh_Plane = {
  .position = mesh_Plane_position,
  .position_length = (sizeof (mesh_Plane_position)) / (sizeof (mesh_Plane_position[0])),
  .normal = mesh_Plane_normal,
  .normal_length = (sizeof (mesh_Plane_normal)) / (sizeof (mesh_Plane_normal[0])),
  .polygon_normal = mesh_Plane_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_Plane_polygon_normal)) / (sizeof (mesh_Plane_polygon_normal[0])),
  .polygons = mesh_Plane_polygons,
  .polygons_length = (sizeof (mesh_Plane_polygons)) / (sizeof (mesh_Plane_polygons[0])),
  .uv_layers = mesh_Plane_uv_layers,
  .uv_layers_length = (sizeof (mesh_Plane_uv_layers)) / (sizeof (mesh_Plane_uv_layers[0])),
};

const vec3 mesh_containercubemesh1_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_containercubemesh1_UVMap_uvmap[] = {
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
};

const vec2 mesh_containercubemesh1_lightmap_uvmap[] = {
  {0.831329, 0.668671},
  {0.668671, 0.668671},
  {0.668671, 0.831329},
  {0.831329, 0.831329},
  {0.997996, 0.002004},
  {0.835337, 0.002004},
  {0.835337, 0.164663},
  {0.997996, 0.164663},
  {0.164663, 0.835337},
  {0.002004, 0.835337},
  {0.002004, 0.997996},
  {0.164663, 0.997996},
  {0.997996, 0.168671},
  {0.835337, 0.168671},
  {0.835337, 0.331329},
  {0.997996, 0.331329},
  {0.331329, 0.835337},
  {0.168671, 0.835337},
  {0.168671, 0.997996},
  {0.331329, 0.997996},
  {0.497996, 0.835337},
  {0.335337, 0.835337},
  {0.335337, 0.997996},
  {0.497996, 0.997996},
};

const vec3 mesh_containercubemesh1_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_containercubemesh1_polygon_normal[] = {
  {0.000000, 1.000000, 0.000000},
  {-1.000000, 0.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_containercubemesh1_polygons[] = {
  {7, 6, 2, 3},
  {1, 3, 2, 0},
  {5, 4, 6, 7},
  {1, 0, 4, 5},
  {0, 2, 6, 4},
  {3, 1, 5, 7},
};

const vec2 * mesh_containercubemesh1_uv_layers[] = {
  mesh_containercubemesh1_UVMap_uvmap,
  mesh_containercubemesh1_lightmap_uvmap,
};

const mesh mesh_containercubemesh1 = {
  .position = mesh_containercubemesh1_position,
  .position_length = (sizeof (mesh_containercubemesh1_position)) / (sizeof (mesh_containercubemesh1_position[0])),
  .normal = mesh_containercubemesh1_normal,
  .normal_length = (sizeof (mesh_containercubemesh1_normal)) / (sizeof (mesh_containercubemesh1_normal[0])),
  .polygon_normal = mesh_containercubemesh1_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_containercubemesh1_polygon_normal)) / (sizeof (mesh_containercubemesh1_polygon_normal[0])),
  .polygons = mesh_containercubemesh1_polygons,
  .polygons_length = (sizeof (mesh_containercubemesh1_polygons)) / (sizeof (mesh_containercubemesh1_polygons[0])),
  .uv_layers = mesh_containercubemesh1_uv_layers,
  .uv_layers_length = (sizeof (mesh_containercubemesh1_uv_layers)) / (sizeof (mesh_containercubemesh1_uv_layers[0])),
};

const vec3 mesh_lightcubemesh_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_lightcubemesh_UVMap_uvmap[] = {
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {0.000000, 0.000000},
  {1.000000, 0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
};

const vec3 mesh_lightcubemesh_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_lightcubemesh_polygon_normal[] = {
  {-1.000000, 0.000000, 0.000000},
  {0.000000, 1.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_lightcubemesh_polygons[] = {
  {0, 1, 3, 2},
  {2, 3, 7, 6},
  {6, 7, 5, 4},
  {4, 5, 1, 0},
  {2, 6, 4, 0},
  {7, 3, 1, 5},
};

const vec2 * mesh_lightcubemesh_uv_layers[] = {
  mesh_lightcubemesh_UVMap_uvmap,
};

const mesh mesh_lightcubemesh = {
  .position = mesh_lightcubemesh_position,
  .position_length = (sizeof (mesh_lightcubemesh_position)) / (sizeof (mesh_lightcubemesh_position[0])),
  .normal = mesh_lightcubemesh_normal,
  .normal_length = (sizeof (mesh_lightcubemesh_normal)) / (sizeof (mesh_lightcubemesh_normal[0])),
  .polygon_normal = mesh_lightcubemesh_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_lightcubemesh_polygon_normal)) / (sizeof (mesh_lightcubemesh_polygon_normal[0])),
  .polygons = mesh_lightcubemesh_polygons,
  .polygons_length = (sizeof (mesh_lightcubemesh_polygons)) / (sizeof (mesh_lightcubemesh_polygons[0])),
  .uv_layers = mesh_lightcubemesh_uv_layers,
  .uv_layers_length = (sizeof (mesh_lightcubemesh_uv_layers)) / (sizeof (mesh_lightcubemesh_uv_layers[0])),
};

const vec3 mesh_containercubemesh2_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_containercubemesh2_UVMap_uvmap[] = {
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
};

const vec2 mesh_containercubemesh2_lightmap_uvmap[] = {
  {0.164663, 0.002004},
  {0.002004, 0.002004},
  {0.002004, 0.164663},
  {0.164663, 0.164663},
  {0.164663, 0.168671},
  {0.002004, 0.168671},
  {0.002004, 0.331329},
  {0.164663, 0.331329},
  {0.331329, 0.002004},
  {0.168671, 0.002004},
  {0.168671, 0.164663},
  {0.331329, 0.164663},
  {0.331329, 0.168671},
  {0.168671, 0.168671},
  {0.168671, 0.331329},
  {0.331329, 0.331329},
  {0.164663, 0.335337},
  {0.002004, 0.335337},
  {0.002004, 0.497996},
  {0.164663, 0.497996},
  {0.497996, 0.002004},
  {0.335337, 0.002004},
  {0.335337, 0.164663},
  {0.497996, 0.164663},
};

const vec3 mesh_containercubemesh2_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_containercubemesh2_polygon_normal[] = {
  {0.000000, 1.000000, 0.000000},
  {-1.000000, 0.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_containercubemesh2_polygons[] = {
  {7, 6, 2, 3},
  {1, 3, 2, 0},
  {5, 4, 6, 7},
  {1, 0, 4, 5},
  {0, 2, 6, 4},
  {3, 1, 5, 7},
};

const vec2 * mesh_containercubemesh2_uv_layers[] = {
  mesh_containercubemesh2_UVMap_uvmap,
  mesh_containercubemesh2_lightmap_uvmap,
};

const mesh mesh_containercubemesh2 = {
  .position = mesh_containercubemesh2_position,
  .position_length = (sizeof (mesh_containercubemesh2_position)) / (sizeof (mesh_containercubemesh2_position[0])),
  .normal = mesh_containercubemesh2_normal,
  .normal_length = (sizeof (mesh_containercubemesh2_normal)) / (sizeof (mesh_containercubemesh2_normal[0])),
  .polygon_normal = mesh_containercubemesh2_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_containercubemesh2_polygon_normal)) / (sizeof (mesh_containercubemesh2_polygon_normal[0])),
  .polygons = mesh_containercubemesh2_polygons,
  .polygons_length = (sizeof (mesh_containercubemesh2_polygons)) / (sizeof (mesh_containercubemesh2_polygons[0])),
  .uv_layers = mesh_containercubemesh2_uv_layers,
  .uv_layers_length = (sizeof (mesh_containercubemesh2_uv_layers)) / (sizeof (mesh_containercubemesh2_uv_layers[0])),
};

const vec3 mesh_containercubemesh3_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_containercubemesh3_UVMap_uvmap[] = {
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
};

const vec2 mesh_containercubemesh3_lightmap_uvmap[] = {
  {0.831329, 0.168671},
  {0.668671, 0.168671},
  {0.668671, 0.331329},
  {0.831329, 0.331329},
  {0.331329, 0.668671},
  {0.168671, 0.668671},
  {0.168671, 0.831329},
  {0.331329, 0.831329},
  {0.497996, 0.668671},
  {0.335337, 0.668671},
  {0.335337, 0.831329},
  {0.497996, 0.831329},
  {0.831329, 0.335337},
  {0.668671, 0.335337},
  {0.668671, 0.497996},
  {0.831329, 0.497996},
  {0.831329, 0.502004},
  {0.668671, 0.502004},
  {0.668671, 0.664663},
  {0.831329, 0.664663},
  {0.664663, 0.668671},
  {0.502004, 0.668671},
  {0.502004, 0.831329},
  {0.664663, 0.831329},
};

const vec3 mesh_containercubemesh3_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_containercubemesh3_polygon_normal[] = {
  {0.000000, 1.000000, 0.000000},
  {-1.000000, 0.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_containercubemesh3_polygons[] = {
  {7, 6, 2, 3},
  {1, 3, 2, 0},
  {5, 4, 6, 7},
  {1, 0, 4, 5},
  {0, 2, 6, 4},
  {3, 1, 5, 7},
};

const vec2 * mesh_containercubemesh3_uv_layers[] = {
  mesh_containercubemesh3_UVMap_uvmap,
  mesh_containercubemesh3_lightmap_uvmap,
};

const mesh mesh_containercubemesh3 = {
  .position = mesh_containercubemesh3_position,
  .position_length = (sizeof (mesh_containercubemesh3_position)) / (sizeof (mesh_containercubemesh3_position[0])),
  .normal = mesh_containercubemesh3_normal,
  .normal_length = (sizeof (mesh_containercubemesh3_normal)) / (sizeof (mesh_containercubemesh3_normal[0])),
  .polygon_normal = mesh_containercubemesh3_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_containercubemesh3_polygon_normal)) / (sizeof (mesh_containercubemesh3_polygon_normal[0])),
  .polygons = mesh_containercubemesh3_polygons,
  .polygons_length = (sizeof (mesh_containercubemesh3_polygons)) / (sizeof (mesh_containercubemesh3_polygons[0])),
  .uv_layers = mesh_containercubemesh3_uv_layers,
  .uv_layers_length = (sizeof (mesh_containercubemesh3_uv_layers)) / (sizeof (mesh_containercubemesh3_uv_layers[0])),
};

const vec3 mesh_containercubemesh4_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_containercubemesh4_UVMap_uvmap[] = {
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
};

const vec2 mesh_containercubemesh4_lightmap_uvmap[] = {
  {0.497996, 0.168671},
  {0.335337, 0.168671},
  {0.335337, 0.331329},
  {0.497996, 0.331329},
  {0.331329, 0.335337},
  {0.168671, 0.335337},
  {0.168671, 0.497996},
  {0.331329, 0.497996},
  {0.497996, 0.335337},
  {0.335337, 0.335337},
  {0.335337, 0.497996},
  {0.497996, 0.497996},
  {0.164663, 0.502004},
  {0.002004, 0.502004},
  {0.002004, 0.664663},
  {0.164663, 0.664663},
  {0.664663, 0.002004},
  {0.502004, 0.002004},
  {0.502004, 0.164663},
  {0.664663, 0.164663},
  {0.331329, 0.502004},
  {0.168671, 0.502004},
  {0.168671, 0.664663},
  {0.331329, 0.664663},
};

const vec3 mesh_containercubemesh4_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_containercubemesh4_polygon_normal[] = {
  {0.000000, 1.000000, 0.000000},
  {-1.000000, 0.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_containercubemesh4_polygons[] = {
  {7, 6, 2, 3},
  {1, 3, 2, 0},
  {5, 4, 6, 7},
  {1, 0, 4, 5},
  {0, 2, 6, 4},
  {3, 1, 5, 7},
};

const vec2 * mesh_containercubemesh4_uv_layers[] = {
  mesh_containercubemesh4_UVMap_uvmap,
  mesh_containercubemesh4_lightmap_uvmap,
};

const mesh mesh_containercubemesh4 = {
  .position = mesh_containercubemesh4_position,
  .position_length = (sizeof (mesh_containercubemesh4_position)) / (sizeof (mesh_containercubemesh4_position[0])),
  .normal = mesh_containercubemesh4_normal,
  .normal_length = (sizeof (mesh_containercubemesh4_normal)) / (sizeof (mesh_containercubemesh4_normal[0])),
  .polygon_normal = mesh_containercubemesh4_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_containercubemesh4_polygon_normal)) / (sizeof (mesh_containercubemesh4_polygon_normal[0])),
  .polygons = mesh_containercubemesh4_polygons,
  .polygons_length = (sizeof (mesh_containercubemesh4_polygons)) / (sizeof (mesh_containercubemesh4_polygons[0])),
  .uv_layers = mesh_containercubemesh4_uv_layers,
  .uv_layers_length = (sizeof (mesh_containercubemesh4_uv_layers)) / (sizeof (mesh_containercubemesh4_uv_layers[0])),
};

const vec3 mesh_containercubemesh5_position[] = {
  {-1.000000, -1.000000, -1.000000},
  {-1.000000, -1.000000, 1.000000},
  {-1.000000, 1.000000, -1.000000},
  {-1.000000, 1.000000, 1.000000},
  {1.000000, -1.000000, -1.000000},
  {1.000000, -1.000000, 1.000000},
  {1.000000, 1.000000, -1.000000},
  {1.000000, 1.000000, 1.000000},
};

const vec2 mesh_containercubemesh5_UVMap_uvmap[] = {
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {1.000000, 1.000000},
  {0.000000, 1.000000},
  {-0.000000, 0.000000},
  {1.000000, -0.000000},
};

const vec2 mesh_containercubemesh5_lightmap_uvmap[] = {
  {0.664663, 0.168671},
  {0.502004, 0.168671},
  {0.502004, 0.331329},
  {0.664663, 0.331329},
  {0.497996, 0.502004},
  {0.335337, 0.502004},
  {0.335337, 0.664663},
  {0.497996, 0.664663},
  {0.664663, 0.335337},
  {0.502004, 0.335337},
  {0.502004, 0.497996},
  {0.664663, 0.497996},
  {0.664663, 0.502004},
  {0.502004, 0.502004},
  {0.502004, 0.664663},
  {0.664663, 0.664663},
  {0.164663, 0.668671},
  {0.002004, 0.668671},
  {0.002004, 0.831329},
  {0.164663, 0.831329},
  {0.831329, 0.002004},
  {0.668671, 0.002004},
  {0.668671, 0.164663},
  {0.831329, 0.164663},
};

const vec3 mesh_containercubemesh5_normal[] = {
  {-0.577350, -0.577350, -0.577350},
  {-0.577350, -0.577350, 0.577350},
  {-0.577350, 0.577350, -0.577350},
  {-0.577350, 0.577350, 0.577350},
  {0.577350, -0.577350, -0.577350},
  {0.577350, -0.577350, 0.577350},
  {0.577350, 0.577350, -0.577350},
  {0.577350, 0.577350, 0.577350},
};

const vec3 mesh_containercubemesh5_polygon_normal[] = {
  {0.000000, 1.000000, 0.000000},
  {-1.000000, 0.000000, 0.000000},
  {1.000000, 0.000000, 0.000000},
  {0.000000, -1.000000, 0.000000},
  {0.000000, 0.000000, -1.000000},
  {0.000000, 0.000000, 1.000000},
};

const polygon mesh_containercubemesh5_polygons[] = {
  {7, 6, 2, 3},
  {1, 3, 2, 0},
  {5, 4, 6, 7},
  {1, 0, 4, 5},
  {0, 2, 6, 4},
  {3, 1, 5, 7},
};

const vec2 * mesh_containercubemesh5_uv_layers[] = {
  mesh_containercubemesh5_UVMap_uvmap,
  mesh_containercubemesh5_lightmap_uvmap,
};

const mesh mesh_containercubemesh5 = {
  .position = mesh_containercubemesh5_position,
  .position_length = (sizeof (mesh_containercubemesh5_position)) / (sizeof (mesh_containercubemesh5_position[0])),
  .normal = mesh_containercubemesh5_normal,
  .normal_length = (sizeof (mesh_containercubemesh5_normal)) / (sizeof (mesh_containercubemesh5_normal[0])),
  .polygon_normal = mesh_containercubemesh5_polygon_normal,
  .polygon_normal_length = (sizeof (mesh_containercubemesh5_polygon_normal)) / (sizeof (mesh_containercubemesh5_polygon_normal[0])),
  .polygons = mesh_containercubemesh5_polygons,
  .polygons_length = (sizeof (mesh_containercubemesh5_polygons)) / (sizeof (mesh_containercubemesh5_polygons[0])),
  .uv_layers = mesh_containercubemesh5_uv_layers,
  .uv_layers_length = (sizeof (mesh_containercubemesh5_uv_layers)) / (sizeof (mesh_containercubemesh5_uv_layers[0])),
};

const struct object objects[] = {
  { // object_Plane
    .mesh = &mesh_Plane,
    .scale = {5.000000, 5.000000, 1.000000},
    .rotation = {0.000000, 0.000000, 0.000000, 1.000000}, // quaternion (XYZW)
    .location = {0.000000, 0.000000, 0.000000},
  },
  { // object_containercube1
    .mesh = &mesh_containercubemesh1,
    .scale = {0.500000, 0.500000, 0.500000},
    .rotation = {-0.029408, 0.323142, -0.128470, 0.937129}, // quaternion (XYZW)
    .location = {-0.913651, 0.474673, 0.796012},
  },
  { // object_containercube2
    .mesh = &mesh_containercubemesh2,
    .scale = {0.433165, 0.433164, 0.433165},
    .rotation = {0.167269, 0.020963, -0.062335, 0.983715}, // quaternion (XYZW)
    .location = {1.959044, 0.268435, 0.403276},
  },
  { // object_containercube3
    .mesh = &mesh_containercubemesh3,
    .scale = {0.433165, 0.433165, 0.433165},
    .rotation = {0.043599, 0.204523, -0.282546, 0.936182}, // quaternion (XYZW)
    .location = {0.979381, -0.585301, -0.197909},
  },
  { // object_containercube4
    .mesh = &mesh_containercubemesh4,
    .scale = {0.201927, 0.201927, 0.201927},
    .rotation = {0.076094, 0.080903, -0.107096, 0.988026}, // quaternion (XYZW)
    .location = {0.851650, 0.413418, 1.370947},
  },
  { // object_containercube5
    .mesh = &mesh_containercubemesh5,
    .scale = {0.201927, 0.201927, 0.201927},
    .rotation = {0.073075, -0.010083, -0.084328, 0.993704}, // quaternion (XYZW)
    .location = {0.242747, 0.769229, 0.513025},
  },
  { // object_bluecube
    .mesh = &mesh_lightcubemesh,
    .scale = {0.200000, 0.200000, 0.200000},
    .rotation = {-0.131058, 0.376347, -0.164256, 0.902334}, // quaternion (XYZW)
    .location = {-0.388222, 0.468224, 1.569220},
  },
  { // object_greencube
    .mesh = &mesh_lightcubemesh,
    .scale = {0.150812, 0.150812, 0.150812},
    .rotation = {0.000000, 0.000000, 0.000000, 1.000000}, // quaternion (XYZW)
    .location = {0.448543, 2.267525, 0.599414},
  },
  { // object_redcube
    .mesh = &mesh_lightcubemesh,
    .scale = {0.100338, 0.100338, 0.100338},
    .rotation = {0.080010, -0.229758, 0.100278, 0.964756}, // quaternion (XYZW)
    .location = {0.606424, 0.150943, 1.609826},
  },
  { // object_whitecube
    .mesh = &mesh_lightcubemesh,
    .scale = {0.150812, 0.150812, 0.150812},
    .rotation = {0.000000, 0.000000, 0.000000, 1.000000}, // quaternion (XYZW)
    .location = {1.715308, 0.016594, 1.375648},
  },
};

