#ifndef __KERNEL_GRAPHICS_TTY_H__
#define __KERNEL_GRAPHICS_TTY_H__

#include <stdint.h>
void init_text(uint8_t*);
void tty_putc(char);
#endif
