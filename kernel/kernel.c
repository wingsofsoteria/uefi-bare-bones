typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long int uint64_t;

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %b0, %1" : : "a"(val), "Nd"(port) : "memory");
  /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers
   * that fit in 8b. (N constraint). Wider immediate constants would be
   * truncated at assemble-time (e.g. "i" constraint). The  outb  %al, %dx
   * encoding is the only option for all other cases. %1 expands to %dx because
   * port  is a uint16_t.  %w1 could be used if we had the port number a wider C
   * type */
}

void qemu_print(char* text) {
#ifndef QEMU_DEBUG
  return;
#endif
  while (*text != 0) {
    outb(0xE9, *text);
    text++;
  }
  outb(0xE9, '\n');
}

void test_pixels(uint32_t base, uint32_t pitch, int start_x, int start_y) {
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 32; j++) {
      *(uint32_t*)(base + pitch * (start_y + j) + 4 * (start_x + i)) =
          (0x00FF0000) >> (((i / 4) - 1) * 8);
    }
  }
}
void reverse(char str[], int length) {
  int start = 0;
  int end = length - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    end--;
    start++;
  }
}
char* inttostr(uint32_t num, char* str) {
  int i = 0;
  int negative = 1;
  if (num == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }
  if (num < 0) {
    negative = 0;
    num = -num;
  }
  while (num != 0) {
    int rem = num % 10;
    str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
    num = num / 10;
  }
  if (negative) {
    str[i++] = '-';
  }
  str[i] = '\0';
  reverse(str, i);
  return str;
}

uint64_t _start(uint64_t base, uint32_t horizontal_res, uint32_t vertical_res,
                uint32_t pitch, char magic[5]) {
  qemu_print("Begin QEMU Debug");
  qemu_print(magic);
  test_pixels(base, pitch, horizontal_res / 4, vertical_res / 4);
  return base;
}
