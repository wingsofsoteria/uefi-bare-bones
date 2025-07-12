#include <string.h>

int memcmp(const void* aptr, const void* bptr, size_t size)
{
  const unsigned char* a = (const unsigned char*)aptr;
  const unsigned char* b = (const unsigned char*)bptr;
  for (size_t i = 0; i < size; i++)
  {
    if (a[i] < b[i])
    {
      return -1;
    }
    else if (a[i] > b[i])
    {
      return 1;
    }
  }
  return 0;
}

void* memmove(void* dst, void* src, size_t size)
{
  unsigned char* char_dst = (unsigned char*)dst;
  unsigned char* char_src = (unsigned char*)src;
  if (char_dst < char_src)
  {
    for (size_t i = 0; i < size; i++)
    {
      char_dst[i] = char_src[i];
    }
  }
  else
  {
    for (size_t i = size; i != 0; i--)
    {
      char_dst[i - 1] = char_src[i - 1];
    }
  }
  return dst;
}

size_t strlen(const char* str)
{
  size_t len = 0;
  while (*(str + len))
  {
    len++;
  }
  return len;
}

void* memset(void* mem, int val, size_t size)
{
  unsigned char* buf = (unsigned char*)mem;
  for (size_t i = 0; i < size; i++)
  {
    buf[i] = val;
  }
  return mem;
}
