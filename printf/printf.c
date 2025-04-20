#include <stdint.h>
#include <stdarg.h>

#include "parse.h"
#include "unparse.h"
#include "printf.h"
//#include "sh7091_scif.h"

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
  if (*format >= '1' && *format <= '9')
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

void print_string(const char * s, int length)
{
  for (int i = 0; i < length; i++) {
    print_char(s[i]);
  }
}

void print_bytes(const uint8_t * s, int length)
{
  for (int i = 0; i < length; i++) {
    print_char(s[i]);
  }
}

void print_chars(const uint16_t * s, int length)
{
  for (int i = 0; i < length; i++) {
    print_char(s[i]);
  }
}

void print_cstring(const char * s)
{
  while (*s != 0) {
    print_char(*s++);
  }
}

void _printf(const char * format, ...)
{
  va_list args;
  va_start(args, format);

  while (true) {
    if (*format == 0)
      break;

    switch (*format) {
    case '%':
      {
        struct format ft = {0};
        format = parse_escape(format + 1, &ft);
        switch (ft.type) {
        case FORMAT_BASE10_UNSIGNED:
          {
            uint32_t num = va_arg(args, uint32_t);
            char s[10];
            int offset = unparse_base10_unsigned(s, num, ft.pad_length, ft.fill_char);
            print_string(s, offset);
          }
          break;
        case FORMAT_BASE10:
          {
            int32_t num = va_arg(args, int32_t);
            char s[10];
            int offset = unparse_base10(s, num, ft.pad_length, ft.fill_char);
            print_string(s, offset);
          }
          break;
        case FORMAT_BASE10_64:
          {
            int64_t num = va_arg(args, int64_t);
            char s[20];
            int offset = unparse_base10_64(s, num, ft.pad_length, ft.fill_char);
            print_string(s, offset);
          }
          break;
        case FORMAT_POINTER:
          {
            print_char('0');
            print_char('x');
          }
          [[fallthrough]];
        case FORMAT_BASE16:
          {
            uint32_t num = va_arg(args, uint32_t);
            char s[8];
            int offset = unparse_base16(s, num, ft.pad_length, ft.fill_char);
            print_string(s, offset);
          }
          break;
        case FORMAT_STRING:
          {
            const char * s = va_arg(args, const char *);
            while (*s != 0) {
              char c = *s++;
              print_char(c);
            }
          }
          break;
        case FORMAT_CHAR:
          {
            const int c = va_arg(args, const int);
            print_char((char)c);
          }
          break;
        case FORMAT_PERCENT:
          print_char('%');
          break;
        }
      }
      break;
    default:
      {
        char c = *format++;
        print_char(c);
      }
      break;
    }
  }

  va_end(args);
}
