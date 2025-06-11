#ifndef __EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_H__
#define __EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_H__

#include "types.h"

struct efi_simple_text_output_protocol {
  void (*unused1)(void);

  efi_status_t (*output_string)(struct efi_simple_text_output_protocol *,
                                uint16_t *);

  void (*unused2)(void);
  void (*unused3)(void);
  void (*unused4)(void);
  void (*unused5)(void);

  efi_status_t (*clear_screen)(struct efi_simple_text_output_protocol *);

  void (*unused6)(void);
  void (*unused7)(void);

  void *unused8;
};

#endif  // __EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_H__
