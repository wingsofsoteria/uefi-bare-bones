#ifndef __EFI_GRAPHICS_OUTPUT_PROTOCOL_H__
#define __EFI_GRAPHICS_OUTPUT_PROTOCOL_H__

#include <stdint.h>

#include "types.h"
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}

enum efi_graphics_pixel_format {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
};

struct efi_pixel_bitmask {
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
  uint32_t reserved_mask;
};

struct efi_graphics_output_protocol_information {
  uint32_t version;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  enum efi_graphics_pixel_format pixel_format;
  struct efi_pixel_bitmask pixel_information;
  uint32_t pixels_per_scanline;
};
struct efi_graphics_output_protocol_mode {
  uint32_t max_mode;
  uint32_t mode;
  struct efi_graphics_output_protocol_information* info;
  uint64_t size_of_info;
  uint64_t frame_buffer_base;
  uint64_t frame_buffer_size;
};
enum efi_gop_blt_operation {
  EfiBltVideoFill,
  EfiBltVideoToBltBuffer,
  EfiBltBufferToVideo,
  EfiBltVideoToVideo,
  EfiGraphicsOutputBltOperationMax
};
struct efi_graphics_output_protocol {
  efi_status_t (*query_mode)(struct efi_graphics_output_protocol*, uint32_t,
                             uint64_t*);
  efi_status_t (*set_mode)(struct efi_graphics_output_protocol*, uint32_t);
  efi_status_t (*blt)(struct efi_graphics_output_protocol*, uint32_t*,
                      enum efi_gop_blt_operation, uint64_t, uint64_t, uint64_t,
                      uint64_t, uint64_t, uint64_t, uint64_t);
  struct efi_graphics_output_protocol_mode* mode;
};

#endif
