#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "q3bsp.h"

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

void print_header(struct q3bsp_header * header)
{
  printf("magic: ");
  putchar(header->magic[0]);
  putchar(header->magic[1]);
  putchar(header->magic[2]);
  putchar(header->magic[3]);
  putchar('\n');
  printf("version: %x\n", header->version);

  print_direntries(header);
}

void print_textures(uint8_t * buf, struct q3bsp_header * header)
{
  q3bsp_direntry * te = &header->direntries[LUMP_TEXTURES];
  q3bsp_texture_t * textures = reinterpret_cast<q3bsp_texture_t *>(&buf[te->offset]);

  int count = te->length / (sizeof (struct q3bsp_texture));

  for (int i = 0; i < count; i++) {
    q3bsp_texture_t * texture = &textures[i];
    printf("texture [%d]\n", i);
    printf("  name=%s\n", texture->name);
    printf("  flags=%x\n", texture->flags);
    printf("  contents=%x\n", texture->contents);
  }
}

void print_models(uint8_t * buf, struct q3bsp_header * header)
{
  q3bsp_direntry * me = &header->direntries[LUMP_MODELS];
  q3bsp_model_t * models = reinterpret_cast<q3bsp_model_t *>(&buf[me->offset]);

  int count = me->length / (sizeof (struct q3bsp_model));

  for (int i = 0; i < count; i++) {
    q3bsp_model_t * model = &models[i];
    printf("model [%d]\n", i);
    printf("  mins={%f, %f, %f}\n", model->mins[0], model->mins[2], model->mins[2]);
    printf("  maxs={%f, %f, %f}\n", model->maxs[0], model->maxs[2], model->maxs[2]);
    printf("  face=%d\n", model->face);
    printf("  n_faces=%d\n", model->n_faces);
    printf("  brush=%d\n", model->brush);
    printf("  n_brushes=%d\n", model->n_brushes);
  }
}

void print_vertexes(uint8_t * buf, struct q3bsp_header * header)
{
  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_vertex_t * vertexes = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);

  int count = ve->length / (sizeof (struct q3bsp_vertex));

  printf("vertexes count: %d\n", count);
  for (int i = 0; i < count; i++) {
    q3bsp_vertex_t * vertex = &vertexes[i];
    printf("vertex [%d]: lightmapcoord=(%f %f)\n", i, vertex->lightmapcoord[0], vertex->lightmapcoord[1]);
    //assert(vertex->lightmapcoord[0] >= 0.0 && vertex->lightmapcoord[0] <= 1.0);
    //assert(vertex->lightmapcoord[1] >= 0.0 && vertex->lightmapcoord[1] <= 1.0);
  }
}

void print_faces(uint8_t * buf, struct q3bsp_header * header)
{
  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_vertex_t * vertexes = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);

  int count = fe->length / (sizeof (struct q3bsp_face));

  //printf("faces count: %d\n", count);
  for (int face_ix = 0; face_ix < count; face_ix++) {
    q3bsp_face_t * face = &faces[face_ix];
    if (face->type != 2)
      continue;
    //printf("face [%d]\n", face_ix);
    //printf("  type=%d n_vertexes=%d n_meshverts=%d texture=%d lightmap=%d size=(%d,%d)\n", face->type, face->n_vertexes, face->n_meshverts, face->texture, face->lm_index, face->size[0], face->size[1]);


    //printf("[");
    printf("(%d, %d)", face->size[0], face->size[1]);
    /*
    for (int i = 0 ; i < face->n_vertexes; i++) {
      q3bsp_vertex_t * vertex = &vertexes[face->vertex + i];
      printf("(%.00f,%.00f,%.00f)", (vertex->position[0]), (vertex->position[1]), (vertex->position[2]));
      if (i < face->n_vertexes - 1)
        printf(",");
    }
    */
    //printf("],");
    printf("\n");
  }
}

void print_lightmaps(uint8_t * buf, struct q3bsp_header * header)
{
  q3bsp_direntry * lme = &header->direntries[LUMP_LIGHTMAPS];
  q3bsp_lightmap_t * lightmaps = reinterpret_cast<q3bsp_lightmap_t *>(&buf[lme->offset]);
  int count = lme->length / (sizeof (struct q3bsp_lightmap));
  printf("lightmaps count: %d offset: %d\n", count, lme->offset);
}

void debug_print_q3bsp(uint8_t * buf, q3bsp_header_t * header)
{
  // header
  //print_header(header);

  if (0) {
    print_textures(buf, header);
  }

  if (0) {
    print_models(buf, header);
  }

  if (0) {
    print_vertexes(buf, header);
  }

  if (1) {
    print_faces(buf, header);
  }

  if (0) {
    print_lightmaps(buf, header);
  }

  if (0) {
    q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
    q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);
    int face_count = fe->length / (sizeof (struct q3bsp_face));

    q3bsp_direntry * te = &header->direntries[LUMP_TEXTURES];
    q3bsp_texture_t * textures = reinterpret_cast<q3bsp_texture_t *>(&buf[te->offset]);
    int texture_count = te->length / (sizeof (struct q3bsp_texture));

    int texture_uses[texture_count] = {};

    for (int i = 0; i < face_count; i++) {
      q3bsp_face_t * face = &faces[i];
      texture_uses[face->texture] += 1;
    }

    for (int i = 0; i < texture_count; i++) {
      q3bsp_texture_t * texture = &textures[i];
      printf("[%d] %s %d %08x\n", i, texture->name, texture_uses[i], texture->flags);
    }
  }
}

int read_file(const char * filename, uint8_t ** buf, uint32_t * size_out)
{
  FILE * file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "fopen(\"%s\", \"rb\"): %s\n", filename, strerror(errno));
    return -1;
  }

  int ret;
  ret = fseek(file, 0L, SEEK_END);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_END)");
    return -1;
  }

  long offset = ftell(file);
  if (offset < 0) {
    fprintf(stderr, "ftell");
    return -1;
  }
  size_t size = offset;

  ret = fseek(file, 0L, SEEK_SET);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_SET)");
    return -1;
  }

  fprintf(stderr, "read_file: %s size %ld\n", filename, size);
  *buf = (uint8_t *)malloc(size);
  size_t fread_size = fread(*buf, 1, size, file);
  if (fread_size != size) {
    fprintf(stderr, "fread `%s` short read: %d ; expected: %d\n", filename, (int)fread_size, (int)size);
    return -1;
  }

  ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "fclose");
    return -1;
  }

  *size_out = size;

  return 0;
}

int main(int argc, const char * argv[])
{
  assert(argc == 2);
  const char * filename = argv[1];

  uint8_t * buf;
  uint32_t size;
  int res = read_file(filename, &buf, &size);
  if (res != 0)
    return EXIT_FAILURE;

  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);
  debug_print_q3bsp(buf, header);

  return EXIT_SUCCESS;
}
