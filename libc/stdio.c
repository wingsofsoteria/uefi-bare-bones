#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#if defined(__is_libk)
  #include <graphics/tty.h>
#endif

// TODO: Implement stdio and the write system call.

int puts(const char* string)
{
  return printf("%s", string);
}

int putchar(int ic)
{
  // #if defined(__is_libk)
  char c = (char)ic;
  tty_putc(c);
#if defined(QEMU_DEBUG)
  outb(0xE9, c);
#endif
  // #else
  // #endif
  return ic;
}

static bool print(const char* data, size_t length)
{
  const unsigned char* bytes = (const unsigned char*)data;
  for (size_t i = 0; i < length; i++)
  {
    if (putchar(bytes[i]) == EOF)
    {
      return false;
    }
  }
  return true;
}

int printf(const char* restrict format, ...)
{
  asm volatile("cli");
  va_list parameters;
  va_start(parameters, format);

  int written      = 0;
  int fmt_len      = -1;
  bool skip_to_fmt = false;
  const char* start_fmt;

  while (*format != '\0')
  {
    size_t maxrem = INT_MAX - written;
    if (!skip_to_fmt)
    {
      if (format[0] != '%' || format[1] == '%')
      {
        if (format[0] == '%')
        {
          format++;
        }
        size_t amount = 1;
        while (format[amount] && format[amount] != '%')
        {
          amount++;
        }
        if (maxrem < amount)
        {
          return -1;
        }
        if (!print(format, amount))
        {
          return -1;
        }
        format  += amount;
        written += amount;
        continue;
      }
      start_fmt = format++;
    }
    skip_to_fmt = false;
    if (*format == 'c')
    {
      format++;
      char c = (char)va_arg(parameters, int);
      if (!maxrem)
      {
        return -1;
      }
      if (!print(&c, sizeof(c)))
      {
        return -1;
      }
      written++;
    }
    else if (*format == 's')
    {
      format++;
      const char* str = va_arg(parameters, const char*);
      size_t len;
      if (fmt_len == -1)
      {
        len = strlen(str);
      }
      else
      {
        len     = fmt_len;
        fmt_len = -1;
      }
      if (maxrem < len)
      {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!print(str, len)) return -1;
      written += len;
    }
    else if (*format == 'x')
    {
      format++;
      unsigned long long int value = va_arg(parameters, unsigned long long int);
      int len;
      int size = numlen(value, 16);
      if (fmt_len == -1)
      {
        len = size;
      }
      else
      {
        len     = fmt_len;
        fmt_len = -1;
      }
      char out[size + 1]; // TODO: GET PROPER SIZE
      itoa(value, out, 16, size);
      if (!print(out, len))
      {
        return -1;
      }
      written += size;
    }
    else if (*format == 'd')
    {
      format++;
      int value = va_arg(parameters, int);
      int size;
      bool is_negative = false;
      if (value < 0)
      {
        size         = numlen(-value, 10);
        size        += 1;
        is_negative  = true;
      }
      else
      {
        size = numlen(value, 10);
      }
      char out[size + 1];
      if (is_negative)
      {
        out[0] = '-';
        itoa(-value, out, 10, size);
      }
      else
      {
        itoa(value, out, 10, size);
      }
      int len;
      if (fmt_len == -1)
      {
        len = size;
      }
      else
      {
        len     = fmt_len;
        fmt_len = -1;
      }
      if (!print(out, len))
      {
        return -1;
      }
      written += size;
    }
    else if (*format == 'b')
    {
      format++;
      unsigned long long int value = va_arg(parameters, unsigned long long int);
      int size                     = numlen(value, 2);
      char out[size + 1]; // TODO: GET PROPER SIZE
      itoa(value, out, 2, size);
      int len;
      if (fmt_len == -1)
      {
        len = size;
      }
      else
      {
        len     = fmt_len;
        fmt_len = -1;
      }
      if (!print(out, len))
      {
        return -1;
      }
      written += size;
    }
    else if (*format == '.')
    {
      skip_to_fmt = true;
      format++;
      fmt_len = 0;
      while (*format > 48 && *format < 58)
      {
        fmt_len += (*format++) - 48;
      }
    }
    else
    {
      format     = start_fmt;
      size_t len = strlen(format);
      if (maxrem < len)
      {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!print(format, len)) return -1;
      written += len;
      format  += len;
    }
  }
  va_end(parameters);
  asm volatile("sti");
  return written;
}
