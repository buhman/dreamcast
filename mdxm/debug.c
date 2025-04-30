#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mdxm.h"

void print_header(struct mdxm_header * header)
{
  printf("magic: ");
  putchar(header->magic[0]);
  putchar(header->magic[1]);
  putchar(header->magic[2]);
  putchar(header->magic[3]);
  putchar('\n');
  printf("version: %x\n", header->version);
}

void print_surface_hierarchy(struct mdxm_header * header)
{
  mdxm_surf_hierarchy_t * sh = (mdxm_surf_hierarchy_t *)(((uint8_t *)header) + header->offset_surface_hierarchy);
  for (int i = 0; i < header->num_surfaces; i++) {
    printf("[%d] %s\n", i, &sh->shader[1]);
    printf("     %s\n", sh->name);
    int offset = (int)(&(((mdxm_surf_hierarchy_t *)0)->child_indexes[sh->num_children]));
    sh = (mdxm_surf_hierarchy_t *)(((uint8_t *)sh) + offset);
  }
}

void print_surfaces(struct mdxm_header * header)
{
  uint8_t * buf = (uint8_t *)header;

  printf("num_lods: %d\n", header->num_lods);
  printf("offset_lods: %d\n", header->offset_lods);
  printf("num_surfaces: %d\n", header->num_surfaces);

  mdxm_lod_t * lod = (mdxm_lod_t *)&buf[header->offset_lods];
  const int surface_offset = (sizeof (mdxm_lod_t)) + (header->num_surfaces * (sizeof (mdxm_lod_surf_offset_t)));
  for (int l = 0; l < header->num_lods; l++) {
    mdxm_surface_t * surf = (mdxm_surface_t *)(((uint8_t *)lod) + surface_offset);
    for (int i = 0; i < header->num_surfaces; i++) {
      //printf("surf ident: %d\n", surf->ident);
      //printf("offset header: %d\n", surf->offset_header);

      mdxm_vertex_t * v = (mdxm_vertex_t *) (((uint8_t *)surf) + surf->offset_verts);
      mdxm_vertex_texture_coord_t * t = (mdxm_vertex_texture_coord_t *)&v[surf->num_verts];
      printf("num_verts %d\n", surf->num_verts);
      for (int j = 0; j < surf->num_verts; j++) {
        printf("[%d] %f %f %f\n", j, v[j].position[0], v[j].position[1], v[j].position[2]);
        printf("     %f %f %f\n", v[j].normal[0], v[j].normal[1], v[j].normal[2]);
      }

      mdxm_triangle_t * triangles = (mdxm_triangle_t *)(((uint8_t *)surf) + surf->offset_triangles);
      printf("num_triangles %d\n", surf->num_triangles);
      for (int j = 0; j < surf->num_triangles; j++) {
        printf("%d %d %d\n", triangles[j].index[0], triangles[j].index[1], triangles[j].index[2]);
      }
      // next surface
      surf = (mdxm_surface_t *)(((uint8_t *)surf) + surf->offset_end);
    }
    // next lod
    lod = (mdxm_lod_t *)(((uint8_t *)lod) + lod->offset_end);
    break;
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

int main(int argc, char * argv[])
{
  assert(argc == 2);
  const char * filename = argv[1];

  uint8_t * buf;
  uint32_t size;
  int res = read_file(filename, &buf, &size);
  if (res != 0)
    return EXIT_FAILURE;

  mdxm_header_t * header = (mdxm_header_t *)(buf);

  print_header(header);
  print_surface_hierarchy(header);
  //print_surfaces(header);
}
