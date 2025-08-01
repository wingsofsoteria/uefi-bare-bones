#include <qemu.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

static int outb(uint16_t port, uint8_t val)
{
  __asm__ volatile("outb %b0, %1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
  /* There's an outb %al, $imm8 encoding, for compile-time constant port
   * numbers that fit in 8b. (N constraint). Wider immediate constants would be
   * truncated at assemble-time (e.g. "i" constraint). The  outb  %al, %dx
   * encoding is the only option for all other cases. %1 expands to %dx because
   * port  is a uint16_t.  %w1 could be used if we had the port number a wider
   * C type */
  return (int)val;
}

static bool print(const char* data, size_t length)
{
  const unsigned char* bytes = (const unsigned char*)data;
  for (size_t i = 0; i < length; i++)
  {
    if (outb(0xE9, bytes[i]) == -1)
    {
      return false;
    }
  }
  return true;
}

int qemu_printf(const char* restrict format, ...)
{
  va_list parameters;
  va_start(parameters, format);

  int written = 0;
  while (*format != '\0')
  {
    size_t maxrem = INT_MAX - written;
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
    const char* start_fmt = format++;
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
      size_t len      = strlen(str);
      if (maxrem < len)
      {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!print(str, len))
        return -1;
      written += len;
    }
    else if (*format == 'x')
    {
      format++;
      unsigned long long int value = va_arg(parameters, unsigned long long int);
      int size                     = numlen(value, 16);
      char out[size + 1]; // TODO: GET PROPER SIZE
      itoa(value, out, 16, size);
      if (!print(out, size))
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
      if (!print(out, size))
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
      if (!print(out, size))
      {
        return -1;
      }
      written += size;
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
      if (!print(format, len))
        return -1;
      written += len;
      format  += len;
    }
  }
  va_end(parameters);
  return written;
}
