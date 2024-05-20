#include "gap_buffer.hpp"

#include "minmax.hpp"
/*
           2   3     4   5   6   7
       s   e
  [a] [ ] [b] [q] | [ ] [ ] [ ] [ ]
               i                 ^
 */

void line_init_from_buf(struct gap_buffer& gb,
			int32_t * offsets,
			int32_t size)
{
  gb.line.offsets = offsets;
  gb.line.size = size;
  gb.line.length = 0;

  for (int32_t i = 0; i < gb.gap_start; i++) {
    if (gb.buf[i] == '\n') {
      gb.line.offsets[gb.line.length++] = i;
    }
  }

  gb.line.gap = gb.line.length;

  for (int32_t i = gb.gap_end; i < gb.size; i++) {
    if (gb.buf[i] == '\n') {
      gb.line.offsets[gb.line.length++] = i;
    }
  }
}

void gap_init_from_buf(struct gap_buffer& gb,
		       char_type * buf,
		       int32_t size,
		       int32_t length)
{
  gb.buf = buf;
  gb.size = size;

  gb.gap_start = length;
  gb.gap_end = size;
}

/*
void gap_resize(struct gap_buffer& gb)
{
  for (int32_t i = (gb.size - 1); i >= gb.gap_end; i--) {
    gb.buf[i + gb.size] = gb.buf[i];
  }
  gb.gap_end += gb.size;
  gb.size += gb.size;
}
*/

void gap_append(struct gap_buffer& gb, char_type c)
{
  if (gb.gap_start == gb.gap_end) {
    return;
    //gap_resize(gb);
  }

  if (c == '\n') {
    for (int32_t i = gb.line.length - 1; i >= gb.line.gap; i--) {
      gb.line.offsets[i + 1] = gb.line.offsets[i];
    }
    gb.line.offsets[gb.line.gap++] = gb.gap_start;
    gb.line.length += 1;
  }
  gb.buf[gb.gap_start++] = c;
}

void gap_pop(struct gap_buffer& gb)
{
  gb.gap_start--;
  if (gb.buf[gb.gap_start] == '\n') {
    for (int32_t i = gb.line.gap; i < gb.line.length; i++) {
      gb.line.offsets[i - 1] = gb.line.offsets[i];
    }
    gb.line.gap -= 1;
    gb.line.length -= 1;
  }
}

/*
   0   1   2   3   4   5   6   7
  [a] [b] [ ] [ ] [ ] [ ] [q] [r]
           ^              e
 */

/*
 */

void gap_cursor_pos(struct gap_buffer& gb, int32_t delta)
{
  if (delta > 0) {
    if (gb.gap_end + delta >= gb.size) {
      delta = gb.size - gb.gap_end;
    }
    for (int32_t i = 0; i < delta; i++) {
      char_type c = gb.buf[gb.gap_end++];
      if (c == '\n') {
	gb.line.offsets[gb.line.gap++] = gb.gap_start;
      }
      gb.buf[gb.gap_start++] = c;
    }
  } else {
    if (gb.gap_start + delta < 0) {
      delta = -gb.gap_start;
    }
    for (int32_t i = delta; i < 0; i++) {
      char_type c = gb.buf[--gb.gap_start];
      gb.buf[--gb.gap_end] = c;
      if (c == '\n') {
	gb.line.offsets[--gb.line.gap] = gb.gap_end;
      }
    }
  }
}

void gap_cursor_pos_abs(struct gap_buffer& gb, int32_t pos)
{
  if (pos > gb.gap_start) {
    pos -= (gb.gap_end - gb.gap_start);
  }
  gap_cursor_pos(gb, pos - gb.gap_start);
}

int32_t gap_column_number(struct gap_buffer& gb)
{
  int32_t line_start = 0;
  if (gb.line.gap > 0)
    line_start = gb.line.offsets[gb.line.gap - 1] + 1;
  return gb.gap_start - line_start;
}

void gap_cursor_pos_line(struct gap_buffer& gb, int32_t delta)
{
  if (delta > 0) {
    int32_t max_pos_delta = gb.line.length - gb.line.gap;
    delta = min(max_pos_delta, delta);
  } else {
    int32_t min_pos_delta = (-gb.line.gap) - 1;
    delta = max(min_pos_delta, delta);
  }
  if (delta == 0)
    return;
  int32_t column = gap_column_number(gb);
  int32_t offset_ix = (gb.line.gap - 1) + delta;
  int32_t buf_ix = 0;
  if (offset_ix >= 0)
    buf_ix = gb.line.offsets[offset_ix] + 1;
  while (buf_ix < gb.size && column > 0 && gb.buf[buf_ix] != '\n') {
    buf_ix += 1;
    column -= 1;
  }
  gap_cursor_pos_abs(gb, buf_ix);
}
