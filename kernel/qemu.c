#include "kernel.h"
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %b0, %1" : : "a"(val), "Nd"(port) : "memory");
  /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers
   * that fit in 8b. (N constraint). Wider immediate constants would be
   * truncated at assemble-time (e.g. "i" constraint). The  outb  %al, %dx
   * encoding is the only option for all other cases. %1 expands to %dx because
   * port  is a uint16_t.  %w1 could be used if we had the port number a wider C
   * type */
}

int qemu_print(char* text) {
#ifndef QEMU_DEBUG
  return 1337;
#else
  while (*text != 0) {
    outb(0xE9, *text);
    text++;
  }
  outb(0xE9, '\n');
#endif
  return 0;
}
