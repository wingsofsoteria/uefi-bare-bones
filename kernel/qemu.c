#include <kernel.h>
static inline void outb(uint16_t port, uint8_t val)
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
}

void qemu_printn(unsigned long long int value, int base)
{
  if (base == 16)
  {
    outb(0xE9, '0');
    outb(0xE9, 'x');
  }
  else if (base == 2)
  {
    outb(0xE9, '0');
    outb(0xE9, 'b');
  }
  if (value == 0)
  {
    qemu_print("0");
    return;
  }
  unsigned long long int copy = value;
  int size                    = 0;
  for (; copy != 0; size++)
  {
    copy /= base; // THIS IS PROBABLY INEFFICIENT BUT IT WORKS FOR NOW
  };
  char out[size + 1]; // TODO: GET PROPER SIZE
  int i  = size;
  out[i] = 0;

  while (value != 0)
  {
    unsigned long long int temp = value % base;
    if (temp < 10)
    {
      temp += 48;
    }
    else
    {
      temp += 55;
    }
    out[--i]  = temp;
    value    /= base;
  }
  qemu_print(out);
}

int qemu_print(char* text)
{
#ifndef QEMU_DEBUG
  return 1337;
#else
  while (*text != 0)
  {
    outb(0xE9, *text);
    text++;
  }
#endif
  return 0;
}
