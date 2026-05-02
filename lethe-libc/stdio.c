#include <stdbool.h>
#include <stdio.h>
#include <string.h>
// TODO: Implement stdio and the write system call.
#if defined(__is_libk)
extern void tty_putc(char);

int putchar(int ic)
{
  char c = (char)ic;
  tty_putc(c);
  return ic;
}
#endif

static int print(const char* data, size_t length)
{
  const unsigned char* bytes = (const unsigned char*)data;
  for (size_t i = 0; i < length; i++)
    {
      if (putchar(bytes[i]) == EOF) return i;
    }
  return length;
}

int puts(const char* string) { return print(string, strlen(string)); }

int printf(const char* restrict format, ...)
{
  va_list parameters;
  va_start(parameters, format);
  int written = vprintf(format, parameters);
  va_end(parameters);
  return written;
}

#define FLAGS_ZEROPAD   (1 << 0)
#define FLAGS_LEFT      (1 << 1)
#define FLAGS_PLUS      (1 << 2)
#define FLAGS_SPACE     (1 << 3)
#define FLAGS_HASH      (1 << 4)
#define FLAGS_UPPER     (1 << 5)
#define FLAGS_CHAR      (1 << 6)
#define FLAGS_SHORT     (1 << 7)
#define FLAGS_LONG      (1 << 8)
#define FLAGS_LONG_LONG (1 << 9)
#define FLAGS_PRECISION (1 << 10)
#define FLAGS_ADAPT_EXP (1 << 11)
#define NTOA_MAX        48

static int is_digit(char c) { return (c >= 0x30) && (c <= 0x39); }

static uint32_t atoi(const char** a)
{
  uint32_t i = 0;
  while (is_digit(**a)) { i = (i * 10) + (uint32_t)((*(*a)++) - 0x30); }
  return i;
}

static size_t print_reverse(
  const char* buf,
  size_t      len,
  uint32_t    flags,
  uint32_t    width,
  size_t      idx
)
{
  size_t start_idx = idx;
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD))
    {
      for (size_t i = len; i < width; i++)
        {
          putchar(' ');
          idx++;
        }
    }
  while (len)
    {
      putchar(buf[--len]);
      idx++;
    }
  if (flags & FLAGS_LEFT)
    {
      while ((idx - start_idx) < width)
        {
          putchar(' ');
          idx++;
        }
    }
  return idx;
}

// NOLINTBEGIN(*-cognitive-complexity)
static size_t __ntoa(char *buf, size_t len, uint32_t flags, uint32_t width,
  uint32_t precision,
  uint32_t base,
  bool     negative,
  size_t   idx
)
{
  if (!(flags & FLAGS_LEFT))
    {
      if (
        width && flags & FLAGS_ZEROPAD &&
        (negative || flags & (FLAGS_PLUS | FLAGS_SPACE))
      )
        {
          width--;
        }
      while (len < precision && len < NTOA_MAX) { buf[len++] = '0'; }
      while (flags & FLAGS_ZEROPAD && len < width && len < NTOA_MAX)
        {
          buf[len++] = '0';
        }
    }

  if (flags & FLAGS_HASH)
    {
      if (
        !(flags & FLAGS_PRECISION) && len && (len == precision || len == width)
      )
        {
          len--;
          if (len && base == 16) { len--; }
        }
      if (base == 16 && !(flags & FLAGS_UPPER) && len < NTOA_MAX)
        {
          buf[len++] = 'x';
        }
      else if (base == 16 && flags & FLAGS_UPPER && len < NTOA_MAX)
        {
          buf[len++] = 'x';
        }
      else if (base == 2 && len < NTOA_MAX) { buf[len++] = 'b'; }
      if (len < NTOA_MAX) { buf[len++] = '0'; }
    }

  if (len < NTOA_MAX)
    {
      if (negative) { buf[len++] = '-'; }
      else if (flags & FLAGS_PLUS) { buf[len++] = '+'; }
      else if (flags & FLAGS_SPACE) { buf[len++] = ' '; }
    }
  return print_reverse(buf, len, flags, width, idx);
}

static size_t ntoa_long(
  unsigned long value,
  uint32_t      flags,
  uint32_t      base,
  uint32_t      width,
  uint32_t      precision,
  size_t        idx,
  bool          negative
)
{
  char   buf[NTOA_MAX];
  size_t len = 0;
  if (!value) flags &= ~FLAGS_HASH;

  if (!(flags & FLAGS_PRECISION) || value)
    {
      do
        {
          const char digit = (char)(value % base);
          buf[len++]       = digit < 10
                               ? digit + '0'
                               : (flags & FLAGS_UPPER ? 'A' : 'a') + digit - 10;
          value           /= base;
        }
      while (value && (len < NTOA_MAX));
    }
  return __ntoa(buf, len, flags, width, precision, base, negative, idx);
}

static size_t ntoa_long_long(
  unsigned long long value,
  uint32_t           flags,
  uint32_t           base,
  uint32_t           width,
  uint32_t           precision,
  size_t             idx,
  bool               negative
)
{
  char   buf[NTOA_MAX];
  size_t len = 0;
  if (!value) flags &= ~FLAGS_HASH;

  if (!(flags & FLAGS_PRECISION) || value)
    {
      do
        {
          const char digit = (char)(value % base);
          buf[len++]       = digit < 10
                               ? digit + '0'
                               : (flags & FLAGS_UPPER ? 'A' : 'a') + digit - 10;
          value           /= base;
        }
      while (value && (len < NTOA_MAX));
    }
  return __ntoa(buf, len, flags, width, precision, base, negative, idx);
}

int vprintf(const char* fmt, va_list va)
{
  uint32_t flags     = 0;
  uint32_t precision = 0;
  uint32_t width     = 0;
  uint32_t n         = 0;
  uint32_t idx       = 0;
  while (*fmt)
    {
      if (*fmt != '%')
        {
          putchar(*fmt);
          idx++;
          fmt++;
          continue;
        }
      //*fmt == '%'
      fmt++;
      do
        {
          switch (*fmt)
            {
              case '0':
                flags |= FLAGS_ZEROPAD;
                fmt++;
                n = 1;
                break;
              case '-':
                flags |= FLAGS_LEFT;
                fmt++;
                n = 1;
                break;
              case '+':
                flags |= FLAGS_PLUS;
                fmt++;
                n = 1;
                break;
              case ' ':
                flags |= FLAGS_SPACE;
                fmt++;
                n = 1;
                break;
              case '#':
                flags |= FLAGS_HASH;
                fmt++;
                n = 1;
                break;
              default: n = 0; break;
            }
        }
      while (n);
      width = 0;
      if (is_digit(*fmt)) { width = atoi(&fmt); }
      else if (*fmt == '*')
        {
          const int width_specifier = va_arg(va, int);
          if (width_specifier < 0)
            {
              flags |= FLAGS_LEFT;
              width  = (uint32_t)-width_specifier;
            }
          else
            {
              width = (uint32_t)width_specifier;
            }
          fmt++;
        }

      precision = 0;
      if (*fmt == '.')
        {
          flags |= FLAGS_PRECISION;
          fmt++;
          if (is_digit(*fmt)) { precision = atoi(&fmt); }
          else if (*fmt == '*')
            {
              const int precision_specifier = va_arg(va, int);
              precision =
                precision_specifier > 0 ? (uint32_t)precision_specifier : 0;
              fmt++;
            }
        }
      switch (*fmt)
        {
          case 'l':
            flags |= FLAGS_LONG;
            fmt++;
            if (*fmt == 'l')
              {
                flags |= FLAGS_LONG_LONG;
                fmt++;
              }
            break;
          case 'h':
            flags |= FLAGS_SHORT;
            fmt++;
            if (*fmt == 'h')
              {
                flags |= FLAGS_CHAR;
                fmt++;
              }
            break;
          case 't':
            flags |=
              sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG;
            fmt++;
            break;
          case 'j':
            flags |=
              sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG;
            fmt++;
            break;
          case 'z':
            flags |=
              sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG;
            fmt++;
            break;
          default: break;
        }
      switch (*fmt)
        {
          case 'd':
          case 'i':
          case 'u':
          case 'x':
          case 'X':
          case 'o':
          case 'b':
            {
              uint32_t base;
              if (*fmt == 'x' || *fmt == 'X') { base = 16; }
              else if (*fmt == 'o') { base = 8; }
              else if (*fmt == 'b') { base = 2; }
              else
                {
                  base   = 10;
                  flags &= ~FLAGS_HASH;
                }

              if (*fmt == 'X') { flags |= FLAGS_UPPER; }

              if ((*fmt != 'i') && (*fmt != 'd'))
                {
                  flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
                }

              if (flags & FLAGS_PRECISION) { flags &= ~FLAGS_ZEROPAD; }
              if ((*fmt == 'i') || (*fmt == 'd'))
                {
                  if (flags & FLAGS_LONG_LONG)
                    {
                      const long long value = va_arg(va, long long);
                      idx                   = ntoa_long_long(
                        (unsigned long long)(value > 0 ? value : 0 - value),
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        value < 0
                      );
                    }
                  else if (flags & FLAGS_LONG)
                    {
                      const long value = va_arg(va, long);
                      idx              = ntoa_long(
                        value > 0 ? value : 0 - value,
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        value < 0
                      );
                    }
                  else
                    {
                      int value = 0;
                      if (flags & FLAGS_CHAR) { value = (char)va_arg(va, int); }
                      else if (flags & FLAGS_SHORT)
                        {
                          value = (short int)va_arg(va, int);
                        }
                      else
                        {
                          value = va_arg(va, int);
                        }
                      idx = ntoa_long(
                        value > 0 ? value : 0 - value,
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        value < 0
                      );
                    }
                }
              else
                {
                  if (flags & FLAGS_LONG_LONG)
                    {
                      idx = ntoa_long_long(
                        va_arg(va, unsigned long long),
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        false
                      );
                    }
                  else if (flags & FLAGS_LONG)
                    {
                      idx = ntoa_long(
                        va_arg(va, unsigned long),
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        false
                      );
                    }
                  else
                    {
                      unsigned int value = 0;
                      if (flags & FLAGS_CHAR)
                        {
                          value = (unsigned char)va_arg(va, unsigned int);
                        }
                      else if (flags & FLAGS_SHORT)
                        {
                          value = (unsigned short int)va_arg(va, unsigned int);
                        }
                      else
                        {
                          value = va_arg(va, unsigned int);
                        }
                      idx = ntoa_long(
                        value,
                        flags,
                        base,
                        width,
                        precision,
                        idx,
                        false
                      );
                    }
                }
              fmt++;
              break;
            }
          case 'c':
            {
              unsigned int l = 1;
              if (!(flags & FLAGS_LEFT))
                {
                  while (l++ < width)
                    {
                      putchar(' ');
                      idx++;
                    }
                }
              putchar((char)va_arg(va, int));
              idx++;
              if (flags & FLAGS_LEFT)
                {
                  while (l++ < width)
                    {
                      putchar(' ');
                      idx++;
                    }
                }
              fmt++;
              break;
            }
          case 's':
            {
              const char*  p = va_arg(va, const char*);
              unsigned int l = strlen(p);
              if (flags & FLAGS_PRECISION)
                {
                  l = l < precision ? l : precision;
                }
              if (!(flags & FLAGS_LEFT))
                {
                  while (l++ < width)
                    {
                      putchar(' ');
                      idx++;
                    }
                }
              while (*p && (!(flags & FLAGS_PRECISION) || precision--))
                {
                  putchar(*(p++));
                  idx++;
                }
              if (flags & FLAGS_LEFT)
                {
                  while (l++ < width)
                    {
                      putchar(' ');
                      idx++;
                    }
                }
              fmt++;
              break;
            }
          case 'p':
            {
              width  = sizeof(void*) * 2;
              flags |= FLAGS_ZEROPAD | FLAGS_UPPER;
              if (sizeof(uintptr_t) == sizeof(long long))
                {
                  idx = ntoa_long_long(
                    (uintptr_t)va_arg(va, void*),
                    flags,
                    16,
                    width,
                    precision,
                    idx,
                    false
                  );
                }
              else
                {
                  idx = ntoa_long(
                    (unsigned long)((uintptr_t)va_arg(va, void*)),
                    flags,
                    16,
                    width,
                    precision,
                    idx,
                    false
                  );
                }
              fmt++;
              break;
            }
          case '%':
            {
              putchar('%');
              idx++;
              fmt++;
              break;
            }
          default:
            {
              putchar(*fmt);
              idx++;
              fmt++;
              break;
            }
        }
    }

  return (int)idx;
}

// NOLINTEND(*-cognitive-complexity)
