#include "kernel.h"
void test_pixels(uint64_t base, uint32_t pitch, int start_x, int start_y) {
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 32; j++) {
      *(uint32_t*)(base + pitch * (start_y + j) + 4 * (start_x + i)) =
          (0x00FF0000) >> (((i / 4) - 1) * 8);
    }
  }
}
