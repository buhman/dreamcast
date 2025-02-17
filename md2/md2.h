struct md2_texture_coordinate
{
  int16_t s;
  int16_t t;
};

struct md2_triangle
{
  uint16_t vertex[3];
  uint16_t st[3];
};

struct md2_vertex
{
  uint8_t v[3];         /* position */
  uint8_t normal_index;
};

struct md2_frame
{
  vec3 scale;               /* scale factor */
  vec3 translate;           /* translation vector */
  char name[16];              /* frame name */
  struct md2_vertex * verts;  /* list of frame's vertices */
};

struct md2_header
{
  int skinwidth;              /* texture width */
  int skinheight;             /* texture height */

  int num_vertices;           /* number of vertices per frame */
  int num_st;                 /* number of texture coordinates */
  int num_tris;               /* number of triangles */
  int num_frames;             /* number of frames */

  struct md2_texture_coordinate * st;
  struct md2_triangle * tris;
  struct md2_frame * frames;
};
