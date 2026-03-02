#include "keyboard.h"
#include "cpu/pit.h"
#include "stdlib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <graphics/tty.h>
#include <scancodes.h>
// TODO find a way to use the loop_print_definition_blocks function without
// hanging the entire system (possible use for tasking?)
// | I just stuck it in the main function for now
char scancode_to_char(uint8_t);
uint8_t KB_STATUS[88] = {0};

void test_ascii_table(char ch, uint8_t byte)
{
  if (ch < 32 || ch > 96) return;
  if (ASCII_SCANCODE_1[ch] != byte)
  {
    printf("character mismatch: %d vs %x\n", ch, ASCII_SCANCODE_1[ch]);
    abort();
  }
}

/* theres gotta be a better way to do this than just waiting
 * basically we just read the keyboard the first time and fail early if the key
 * isn't actually pressed then we wait for up to 500 ms (TODO change the
 * pit_sleep call once we figure out APIC tsc deadline mode) and while waiting
 * we check if the key was released every 10ms */
int naive_key_released(char ascii_key)
{
  if (ascii_key < 32 || ascii_key > 96) return 0;
  uint8_t scancode   = ASCII_SCANCODE_1[ascii_key];
  uint8_t first_read = KB_STATUS[scancode];
  if (!first_read) return 0;
  for (int sleep_counter = 0; sleep_counter < 50; sleep_counter++)
  {
    pit_sleep(10);
    uint8_t second_read = KB_STATUS[scancode];
    if (!second_read) return 1;
  }
  return 0;
}

// the difference between checking if a key was released and if it was pressed
// is downright laughable (hence why I am under the opinion that there must be a
// better way)
int is_key_pressed(char ascii_key)
{
  if (ascii_key < 32 || ascii_key > 96) return 0;
  uint8_t scancode = ASCII_SCANCODE_1[ascii_key];
  return KB_STATUS[scancode];
}

void kb_handle_key()
{
  uint8_t status = inb(0x64);
  if ((status & 0b1) != 1) return;
  uint8_t byte = inb(0x60);

  if (byte <= 0x58)
  {
    KB_STATUS[byte] = 1;
  }
  else if (byte > 0x58 && byte <= 0xD8)
  {
    KB_STATUS[byte - 0x80] = 0;
  }
  char ch = scancode_to_char(byte);
  if (ch != 0)
  {
    putchar(ch);
  }
}

void init_kb_status()
{
  memset(KB_STATUS, 0, 88 * sizeof(uint8_t));
}

char scancode_to_char(uint8_t byte)
{
  uint8_t shift = KB_STATUS[0x36] | KB_STATUS[0x2A];
  char value    = 0;
  if (byte < 58)
  {
    if (shift == 1)
    {
      value = SC1_UPPER_CHARS[byte];
    }
    else
    {
      value = SC1_LOWER_CHARS[byte];
    }
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
