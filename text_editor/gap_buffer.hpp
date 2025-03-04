#pragma once

#include <stdint.h>

typedef uint8_t char_type;

struct line_metadata {
  int32_t * offsets;
  int32_t size;
  int32_t length;
  int32_t gap;
};

struct gap_buffer {
  char_type * buf;
  int32_t size;
  int32_t gap_start;
  int32_t gap_end;
  struct line_metadata line;
};

void line_init_from_buf(struct gap_buffer& gb,
			int32_t * offsets,
			int32_t size);

void gap_init_from_buf(struct gap_buffer& gb,
		       char_type * buf,
		       int32_t size,
		       int32_t length);

void gap_resize(struct gap_buffer& gb);
void gap_append(struct gap_buffer& gb, char_type c);
void gap_pop(struct gap_buffer& gb);
void gap_cursor_pos(struct gap_buffer& gb, int32_t delta);
int32_t gap_column_number(const struct gap_buffer& gb);
void gap_cursor_pos_line(struct gap_buffer& gb, int32_t delta);
