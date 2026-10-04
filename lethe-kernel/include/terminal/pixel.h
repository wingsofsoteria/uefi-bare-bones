#pragma once
#include <stdint.h>

void test_pixels();
void init_fb(void*);
void fill(int start_x, int start_y, int width, int height, uint32_t color);
void put_pixel(int x, int y, uint32_t color);
void clear_screen();
