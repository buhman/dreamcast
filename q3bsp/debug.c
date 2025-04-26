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

void print_header(void * buf)
{
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  printf("magic: ");
  putchar(header->magic[0]);
  putchar(header->magic[1]);
  putchar(header->magic[2]);
  putchar(header->magic[3]);
  putchar('\n');
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
    printf("  type=%d n_vertexes=%d n_meshverts=%d texture=%d\n", face[i].type, face[i].n_vertexes, face[i].n_meshverts, face[i].texture);
  }
}

void debug_print_q3bsp(uint8_t * buf, q3bsp_header_t * header)
{
  // header
  //print_header(buf);

  if (0) {
    q3bsp_direntry * e = &header->direntries[LUMP_TEXTURES];
    print_textures(&buf[e->offset], e->length);
  }

  if (0) {
    q3bsp_direntry * e = &header->direntries[LUMP_MODELS];
    print_models(&buf[e->offset], e->length);
  }

  if (0) {
    q3bsp_direntry * e = &header->direntries[LUMP_FACES];
    print_faces(&buf[e->offset], e->length);
  }

  {
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
      printf("[%d] %s %d\n", i, texture->name, texture_uses[i]);
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
