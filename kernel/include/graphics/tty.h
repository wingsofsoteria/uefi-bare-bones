// clang-format Language: C
#ifndef __KERNEL_GRAPHICS_TTY_H__
#define __KERNEL_GRAPHICS_TTY_H__

#include <stdint.h>
void init_text(uint8_t*);
void tty_putc(char);
void tty_delc();
typedef struct
{
  uint32_t magic;         /* magic bytes to identify PSF */
  uint32_t version;       /* zero */
  uint32_t headersize;    /* offset of bitmaps in file, 32 */
  uint32_t flags;         /* 0 if there's no unicode table */
  uint32_t numglyph;      /* number of glyphs */
  uint32_t bytesperglyph; /* size of each glyph */
  uint32_t height;        /* height in pixels */
  uint32_t width;         /* width in pixels */
} psf_font_t;
#endif
