#include <string.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t size)
{
  char *destp = dest;
  const char *srcp = src;
  for (int i = 0; i < size; i++) {
    destp[i] = srcp[i];
  }

  return dest;
}

int memcmp(const void *aptr, const void *bptr, size_t size)
{
  const unsigned char *a = (const unsigned char *)aptr;
  const unsigned char *b = (const unsigned char *)bptr;
  for (size_t i = 0; i < size; i++) {
    if (a[i] < b[i]) {
      return -1;
    }
    if (a[i] > b[i]) {
      return 1;
    }
  }
  return 0;
}

void *memmove(void *dst, const void *src, size_t size)
{
  unsigned char *char_dst = (unsigned char *)dst;
  unsigned char *char_src = (unsigned char *)src;
  if (char_dst < char_src) {
    for (size_t i = 0; i < size; i++) {
      char_dst[i] = char_src[i];
    }
  } else {
    for (size_t i = size; i != 0; i--) {
      char_dst[i - 1] = char_src[i - 1];
    }
  }
  return dst;
}

size_t strlen(const char *str)
{
  const char *s;
  for (s = str; *s; ++s) {}
  return (size_t)(s - str);
}

size_t strnlen(const char *str, size_t maxlen)
{
  const char *s;
  for (s = str; *s && maxlen--; ++s) {}
  return (size_t)(s - str);
}

int strcmp(const char *s1, const char *s2)
{
  int s1_len = strlen(s1);
  int s2_len = strlen(s2);
  if (s1_len != s2_len) return s1_len - s2_len;
  return strncmp(s1, s2, s1_len);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    if (s1[i] == 0 || s2[i] == 0) {
      break;
    }
    if (s1[i] < s2[i]) {
      return -1;
    }
    if (s1[i] > s2[i]) {
      return 1;
    }
  }
  return 0;
}

void *memset(void *mem, int val, size_t size)
{
  unsigned char *buf = (unsigned char *)mem;
  for (size_t i = 0; i < size; i++) {
    buf[i] = val;
  }
  return mem;
}

/*// assume str is at least char[size + 1]
char *itoa(unsigned long long int number, char *str, int base, int size)
{
  str[size] = 0;
  int i = size;
  if (number == 0) str[--i] = '0';

  while (number != 0) {
    unsigned long long int temp = number % base;
    if (temp < 10) {
      temp += 48;
    } else {
      temp += 55;
    }
    str[--i] = temp;
    number /= base;
  }
  return str;
}

int numlen(unsigned long long int value, int base)
{
  if (value == 0) return 1;

  unsigned long long int copy = value;
  int size = 0;
  for (; copy != 0; size++) {
    copy /= base; // THIS IS PROBABLY INEFFICIENT BUT IT WORKS FOR NOW
  };
  return size;
}*/
