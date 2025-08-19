#include <stdint.h>
#include "graphics/tty.h"
char scancodes[58] = {
  0,
  0,
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '0',
  '-',
  '=',
  0,
  '\t',
  'Q',
  'W',
  'E',
  'R',
  'T',
  'Y',
  'U',
  'I',
  'O',
  'P',
  '[',
  ']',
  '\n',
  0,
  'A',
  'S',
  'D',
  'F',
  'G',
  'H',
  'J',
  'K',
  'L',
  ';',
  '\'',
  '`',
  0,
  '\\',
  'Z',
  'X',
  'C',
  'V',
  'B',
  'N',
  'M',
  ',',
  '.',
  '/',
  0,
  '*',
  0,
  ' ',
};
char scancode_to_char(uint8_t byte)
{
  char value = 0;
  if (byte < 58)
  {
    value = scancodes[byte];
  }
  switch (byte)
  {
    case 0x0E:
      {
        tty_delc();
      }
  }

  return value;
}
