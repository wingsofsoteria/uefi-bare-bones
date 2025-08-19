#include "graphics/tty.h"
#include "graphics/pixel.h"
#include <stdint.h>
extern char _binary_font_psf_start;

int cursor = 1;
int line   = 1;
void init_text(uint8_t* addr)
{
}

void tty_putc(char ch)
{
  if (ch == '\n')
  {
    cursor  = 1;
    line   += 1;
    return;
  }
  if (ch == '\t')
  {
    cursor += 4;
    return;
  }
  if (cursor >= 80)
  {
    cursor  = 1;
    line   += 1;
  }
  if (line >= 25)
  {
    clear_screen();
    cursor = 1;
    line   = 1;
  }
  uint8_t* glyph = (uint8_t*)&_binary_font_psf_start + 32 + ch * 16;

  int mask = 0b10000000;
  for (int j = line * 16; j < (line * 16) + 16; j++)
  {
    for (int i = cursor * 8; i < (cursor * 8) + 8; i++)
    {
      put_pixel(i, j, *glyph & mask ? 0xFFFFFF : 0x00);

      mask >>= 1;
    }
    mask = 0b10000000;
    glyph++;
  }
  cursor++;
}

void tty_delc()
{
  if (cursor == 0)
  {
    if (line == 0)
    {
      tty_putc(' ');
      cursor--;
      return;
    }
    else
    {
      cursor = 79;
      line--;
      tty_putc(' ');
      cursor--;
    }
  }
  else
  {
    cursor--;
    tty_putc(' ');
    cursor--;
  }
}
