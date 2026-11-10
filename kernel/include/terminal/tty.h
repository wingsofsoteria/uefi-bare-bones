#pragma once
#include <stdint.h>

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

void        init_text(uint8_t*);
void        tty_putc(char);
void        tty_delc();
void        set_cursor(uint32_t new_cursor, uint32_t new_line);
uint64_t    get_cursor();
extern char _binary_font_psf_start;
