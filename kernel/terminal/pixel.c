#include "terminal/pixel.h"

#include "loaders/limine.h"
#include "log.h"
#include "stdbool.h"
#include "terminal/tty.h"

#include <stdint.h>

static uint64_t fb_base;
static uint32_t fb_pitch;
static int      fb_yres;
static int      fb_xres;
static bool     initialized = false;

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
  if (!initialized) return;
  for (int x = start_x; x < start_x + width; x++)
    {
      for (int y = start_y; y < start_y + height; y++)
        {
          *(uint32_t*)(fb_base + (fb_pitch * y) + (4 * x)) = color;
        }
    }
}

void clear_screen()
{
  fill(0, 0, fb_xres, fb_yres, 0x0);
  set_cursor(1, 1);
}

void put_pixel(int x, int y, uint32_t color)
{
  if (!initialized) return;
  *(uint32_t*)(fb_base + (fb_pitch * y) + (4 * x)) = color;
}

void init_fb(void* data)
{
  struct limine_framebuffer_response* ptr = data;
  klog("ptr: %p %p\n", ptr, ptr->framebuffers);
  fb_base     = (uint64_t)ptr->framebuffers[0]->address;
  fb_pitch    = ptr->framebuffers[0]->pitch;
  fb_xres     = ptr->framebuffers[0]->width;
  fb_yres     = ptr->framebuffers[0]->height;
  initialized = true;
}

