#include <graphics.h>
uint64_t fb_base;
uint32_t fb_pitch;
int fb_yres;
int fb_xres;
void init_fb(uint64_t base, uint32_t pitch, int x, int y)
{
  fb_base  = base;
  fb_pitch = pitch;
  fb_xres  = x;
  fb_yres  = y;
}

void test_pixels()
{
  int x = fb_xres / 8;
  fill(0, 0, x, fb_yres, 0xFF0000);
  fill(x, 0, x, fb_yres, 0x00FF00);
  fill(x * 2, 0, x, fb_yres, 0x0000FF);
  fill(x * 3, 0, x, fb_yres, 0xFFFF00);
  fill(x * 4, 0, x, fb_yres, 0xFF00FF);
  fill(x * 5, 0, x, fb_yres, 0x00FFFF);
  fill(x * 6, 0, x, fb_yres, 0xFFFFFF);
  fill(x * 7, 0, x, fb_yres, 0x000000);
}

void fill(int start_x, int start_y, int width, int height, uint32_t color)
{
  for (int x = start_x; x < start_x + width; x++)
  {
    for (int y = start_y; y < start_y + height; y++)
    {
      *(uint32_t*)(fb_base + fb_pitch * y + 4 * x) = color;
    }
  }
}

void put_pixel(int x, int y, uint32_t color)
{
  *(uint32_t*)(fb_base + fb_pitch * y + 4 * x) = color;
}
