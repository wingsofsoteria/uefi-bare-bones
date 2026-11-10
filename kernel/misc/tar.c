#include <stdint.h>

struct file
{
  char         filename[100];
  struct file* parent; // can be a directory or another file (like a symlink)
  size_t       size;
  uint8_t      type; // file, directory, symlink, etc
};

struct directory
{
  struct file  header; // contains all the data about this directory
  struct file* contents;
};

// TODO implement file operation functions
