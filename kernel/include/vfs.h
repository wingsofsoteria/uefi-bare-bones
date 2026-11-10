#pragma once

struct fs_driver
{
  void* open;
  void* close;
  void* read;
  void* write;
  void* readdir;
  void* opendir;
}; // TODO ACTUALLY DEFINE THE FUNCTION POINTERS

int               mount(char*, struct fs_driver*);
struct fs_driver* load_fs(char*);
