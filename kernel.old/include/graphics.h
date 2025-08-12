// clang-format Language: C
#ifndef __KERNEL_GRAPHICS_H__
#define __KERNEL_GRAPHICS_H__
#include <stdint.h>
void test_pixels();
void init_fb(uint64_t, uint32_t, int, int);
void fill(int start_x, int start_y, int width, int height, uint32_t color);
void put_pixel(int x, int y, uint32_t color);
void init_text(uint8_t*);
void tty_putc(char);
void clear_screen();
#endif
