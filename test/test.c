#include "test.h"
int _start(char magic[5]) {
  if (magic[0] != 'T' || magic[1] != 'E' || magic[2] != 'S' ||
      magic[3] != 'T' || magic[4] != '5') {
    return 1;
  } else {
    return 10456;
  }
}
