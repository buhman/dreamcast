#include "gap_buffer.hpp"

#include "sh7091/serial.hpp"
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
    serial::integer<uint32_t>(i);
    if (gb.buf[i] == '\n') {
      gb.line.offsets[gb.line.length++] = i;
    }
  }

  gb.line.gap = gb.line.length;

  serial::string("gbgapend\n");
  for (int32_t i = gb.gap_end; i < gb.size; i++) {
    serial::integer<uint32_t>(i);
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
