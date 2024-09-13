#include "ppm.h"

#include <stdio.h>
#include <stdlib.h>

static int advance(uint8_t * buf, int size, int index, uint8_t c)
{
  while (index < size) {
    if (buf[index] == c && buf[index + 1] != c) {
      return index + 1;
    }
    index += 1;
  }
  fprintf(stderr, "end of file: expected `%d`\n", c);
  return -1;
}

int ppm_parse(uint8_t * buf, int size, struct ppm_header * out)
{
  if (size < 2) {
    fprintf(stderr, "file too small: %d\n", size);
    return -1;
  }
  bool magic = buf[0] == 'P' && buf[1] == '6';
  if (!magic) {
    fprintf(stderr, "invalid magic: %c%c\n", buf[0], buf[1]);
    return -1;
  }

  int header[3];
  int header_ix = 0;
  int index = 2;
  uint8_t delimiter = '\n';

  while (header_ix < 3) {
    index = advance(buf, size - index, index, delimiter);
    if (buf[index] == '#')
      continue;

    uint8_t * end;
    int n = strtol((const char *)&buf[index], (char **)&end, 10);
    if (end == buf) {
      fprintf(stderr, "expected integer at index %d\n", index);
      return -1;
    }
    header[header_ix] = n;

    index = end - buf;
    delimiter = header_ix == 0 ? ' ' : '\n';
    header_ix += 1;
  }

  index = advance(buf, size - index, index, '\n');
  out->width = header[0];
  out->height = header[1];
  out->colors = header[2];
  out->data = &buf[index];
  out->length = size - index;

  return 0;
}
