#include <stdint.h>
#include <stdarg.h>

#include "parse.hpp"
#include "unparse.hpp"
#include "printf.hpp"

enum format_type {
  FORMAT_BASE10_UNSIGNED,
  FORMAT_BASE10,
  FORMAT_BASE10_64,
  FORMAT_POINTER,
  FORMAT_BASE16,
  FORMAT_STRING,
  FORMAT_CHAR,
  FORMAT_PERCENT,
};

struct format {
  enum format_type type;
  int pad_length;
  char fill_char;
};

static const char * parse_escape(const char * format, struct format * ft);

static const char * parse_fill_pad(const char * format, struct format * ft)
{
  if (*format == 0)
    return format;
  if (*format >= '1' || *format <= '9')
    ft->fill_char = ' ';
  else
    ft->fill_char = *format++;
  format = parse_base10(format, &ft->pad_length);
  return parse_escape(format, ft);
}

static const char * parse_escape(const char * format, struct format * ft)
{
  switch (*format) {
  case 0:
    return format;
  case 'u':
    ft->type = FORMAT_BASE10_UNSIGNED;
    return format + 1;
  case 'd':
    ft->type = FORMAT_BASE10;
    return format + 1;
  case 'l':
    ft->type = FORMAT_BASE10_64;
    return format + 1;
  case 'p':
    ft->type = FORMAT_POINTER;
    return format + 1;
  case 'x':
    ft->type = FORMAT_BASE16;
    return format + 1;
  case 's':
    ft->type = FORMAT_STRING;
    return format + 1;
  case 'c':
    ft->type = FORMAT_CHAR;
    return format + 1;
  case '%':
    ft->type = FORMAT_PERCENT;
    return format + 1;
  default:
    return parse_fill_pad(format, ft);
  }
}

static inline int copy_string(const char * src, int src_len, char * dst, int dst_len, int dst_ix)
{
  int i;
  for (i = 0; i < src_len; i++) {
    if (dst_ix + i >= dst_len)
      break;
    dst[dst_ix + i] = src[i];
  }
  return i;
}

int snprintf(char * buf, int buf_len, const char * format, ...)
{
  va_list args;
  va_start(args, format);

  int buf_ix = 0;

  while (buf_ix < buf_len) {
    if (*format == 0)
      break;

    switch (*format) {
    case '%':
      {
        struct format ft;
        ft.pad_length = 0;
        ft.fill_char = 0;
        format = parse_escape(format + 1, &ft);
        switch (ft.type) {
        case FORMAT_BASE10_UNSIGNED:
          {
            uint32_t num = va_arg(args, uint32_t);
            char s[10];
            int offset = unparse_base10_unsigned(s, num, ft.pad_length, ft.fill_char);
            buf_ix += copy_string(s, offset, buf, buf_len, buf_ix);
          }
          break;
        case FORMAT_BASE10:
          {
            int32_t num = va_arg(args, int32_t);
            char s[10];
            int offset = unparse_base10(s, num, ft.pad_length, ft.fill_char);
            buf_ix += copy_string(s, offset, buf, buf_len, buf_ix);
          }
          break;
        case FORMAT_BASE10_64:
          {
            int64_t num = va_arg(args, int64_t);
            char s[20];
            int offset = unparse_base10_64(s, num, ft.pad_length, ft.fill_char);
            buf_ix += copy_string(s, offset, buf, buf_len, buf_ix);
          }
          break;
        case FORMAT_POINTER:
          {
            const char s[2] = {'0', 'x'};
            buf_ix += copy_string(s, 2, buf, buf_len, buf_ix);
          }
          /* fall through */;
        case FORMAT_BASE16:
          {
            uint32_t num = va_arg(args, uint32_t);
            char s[8];
            int offset = unparse_base16(s, num, ft.pad_length, ft.fill_char);
            buf_ix += copy_string(s, offset, buf, buf_len, buf_ix);
          }
          break;
        case FORMAT_STRING:
          {
            const char * s = va_arg(args, const char *);
            while (*s != 0 && buf_ix < buf_len) {
              buf[buf_ix++] = *s++;
            }
          }
          break;
        case FORMAT_CHAR:
          {
            const int c = va_arg(args, const int);
            buf[buf_ix++] = c;
          }
          break;
        case FORMAT_PERCENT:
          buf[buf_ix++] = '%';
          break;
        }
      }
      break;
    default:
      {
        char c = *format++;
        buf[buf_ix++] = c;
      }
      break;
    }
  }

  va_end(args);

  return buf_ix;
}
