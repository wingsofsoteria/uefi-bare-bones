// probably gonna use a mountpoint list for the VFS
// type HashMap<String, File System Driver Structure>
// File system driver structure is gonna just be pointers to read, write,
// readdir, opendir, etc

struct vfs_entry
{
  char*             mountpoint;
  struct fs_driver* driver;
};

#include <vfs.h>
#ifdef UTILS_STB
#include "stb_ds.h"

static vfs_entry* vfs_list = NULL;

int mount(char* mountpoint, struct fs_driver* driver)
{
  if (shgeti(vfs_list, mountpoint) >= 0) return -1; // mountpoint exists
  char* key = strdup(mountpoint);
  if (!key) return -1;
  shput(vfs_list, key, driver);
  return 0;
}

struct fs_driver* load_fs(char* mountpoint)
{
  int index = shgeti(vfs_list, mountpoint);
  if (index < 0) return NULL;
  return vfs_list[index];
}
#endif
