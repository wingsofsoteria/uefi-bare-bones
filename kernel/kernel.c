#include "kernel.h"

uint64_t _start(uint64_t base, uint32_t horizontal_res, uint32_t vertical_res,
                uint32_t pitch, char magic[5]) {
  qemu_print("Starting Kernel");
  qemu_print(magic);
  qemu_print("Testing Magic Value");
  test_pixels(base, pitch, horizontal_res / 4, vertical_res / 4);
  return base;
}
