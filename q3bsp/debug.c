void print_direntries(struct q3bsp_header * header)
{
  // direntries
  static const char * lump_name[] = {
    "entities",
    "textures",
    "planes",
    "nodes",
    "leafs",
    "leaffaces",
    "leafbrushes",
    "models",
    "brushes",
    "brushsides",
    "vertexes",
    "meshverts",
    "effects",
    "faces",
    "lightmaps",
    "lightvols",
    "visdata",
  };
  for (int i = 0; i < 17; i++) {
    printf("%s offset=%d length=%d\n",
           lump_name[i],
           header->direntries[i].offset,
           header->direntries[i].length);
  }
}

void print_header(void * buf)
{
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  serial::string("magic: ");
  serial::string((uint8_t *)header->magic, 4);
  serial::character('\n');
  printf("version: %x\n", header->version);

  print_direntries(header);
}

void print_textures(void * buf, int length)
{
  q3bsp_texture_t * texture = reinterpret_cast<q3bsp_texture_t *>(buf);

  int count = length / (sizeof (struct q3bsp_texture));

  for (int i = 0; i < count; i++) {
    printf("texture [%d]\n", i);
    printf("  name=%s\n", texture[i].name);
    printf("  flags=%x\n", texture[i].flags);
    printf("  contents=%x\n", texture[i].contents);
  }
}

void print_models(void * buf, int length)
{
  q3bsp_model_t * model = reinterpret_cast<q3bsp_model_t *>(buf);

  int count = length / (sizeof (struct q3bsp_model));

  for (int i = 0; i < count; i++) {
    printf("model [%d]\n", i);
    printf("  mins={%f, %f, %f}\n", model->mins[0], model->mins[2], model->mins[2]);
    printf("  maxs={%f, %f, %f}\n", model->maxs[0], model->maxs[2], model->maxs[2]);
    printf("  face=%d\n", model->face);
    printf("  n_faces=%d\n", model->n_faces);
    printf("  brush=%d\n", model->brush);
    printf("  n_brushes=%d\n", model->n_brushes);
  }
}

void print_faces(void * buf, int length)
{
  q3bsp_face_t * face = reinterpret_cast<q3bsp_face_t *>(buf);

  int count = length / (sizeof (struct q3bsp_face));

  printf("faces count: %d\n", count);
  for (int i = 0; i < count; i++) {
    printf("face [%d]\n", i);
    printf("  type=%d n_vertexes=%d n_meshverts=%d\n", face[i].type, face[i].n_vertexes, face[i].n_meshverts);
  }
}

void debug_print_q3bsp(uint8_t * buf, q3bsp_header_t * header)
{
  // header
  print_header(buf);

  {
    q3bsp_direntry * e = &header->direntries[LUMP_TEXTURES];
    print_textures(&buf[e->offset], e->length);
  }

  {
    q3bsp_direntry * e = &header->direntries[LUMP_MODELS];
    print_models(&buf[e->offset], e->length);
  }

  {
    q3bsp_direntry * e = &header->direntries[LUMP_FACES];
    print_faces(&buf[e->offset], e->length);
  }
}
