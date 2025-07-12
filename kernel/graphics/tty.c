#include <graphics.h>
char* address;
int cursor = 0;
int line   = 0;
void init_text(char* addr)
{
  address = addr;
}

void tty_putc(char ch)
{
  if (ch == '\n')
  {
    cursor  = 0;
    line   += 1;
    return;
  }
  if (cursor >= 80)
  {
    cursor  = 0;
    line   += 1;
  }

  int y = line * 8;
  int x = cursor * 8;

  int mask = 0b10000000;
  for (int j = 0; j < 8; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      char line = address[(ch * 8) + j];
      if (line & mask)
      {
        put_pixel(x + i, y + j, 0xFFFFFF);
      }
      else
      {
        put_pixel(x + i, y + j, 0x000000);
      }
      mask >>= 1;
    }
    mask = 0b10000000;
  }
  cursor++;
}
