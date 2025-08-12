#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cpio.h"
#include <types.h>
uint8_t* cpio_address;

bool cpio_eof = false;
uint32_t file_size();
void debug_cpio();
void init_cpio(uint8_t* addr)
{
  cpio_address = addr;
  debug_cpio();
}

void debug_cpio()
{
  debug("CPIO HEADER MAGIC %x\n", ((cpio_header_t*)cpio_address)->c_magic);
  debug("CPIO HEADER FILENAME %s\n", (uint8_t*)(cpio_address + CPIO_HEADER_SIZE));
}

bool check_header()
{
  return ((cpio_header_t*)cpio_address)->c_magic == 0070707;
}

void check_eof()
{
  char* filename = (char*)(cpio_address + CPIO_HEADER_SIZE);
  if (strncmp(filename, "TRAILER!!!", file_size()))
  {
    cpio_eof = true;
  }
}
uint8_t* get_file()
{
  cpio_header_t* header = (cpio_header_t*)cpio_address;
  if (!check_header())
  {
    return NULL;
  }
  next_header();
  return (uint8_t*)(header) + CPIO_HEADER_SIZE + header->c_namesize + 1;
}

uint32_t file_size()
{
  return (((cpio_header_t*)cpio_address)->c_filesize[0] << 16) + ((cpio_header_t*)cpio_address)->c_filesize[1];
}

bool next_header()
{
  if (cpio_eof) return false;
  cpio_address = cpio_address + CPIO_HEADER_SIZE + ((cpio_header_t*)cpio_address)->c_namesize + 1 + file_size();
  check_eof();
  debug_cpio();
  return check_header();
}
