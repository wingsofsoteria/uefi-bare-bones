#ifndef __EFI_BOOT_TABLE_H__
#define __EFI_BOOT_TABLE_H__

#include "types.h"

static const uint32_t EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL = 0x00000001;

struct efi_boot_table {
  struct efi_table_header header;

  // Task Priority Services
  void (*unused1)(void);
  void (*unused2)(void);

  // Memory Services
  efi_status_t (*allocate_pages)(enum efi_allocate_type, enum efi_memory_type,
                                 efi_uint_t, uint64_t *);
  efi_status_t (*free_pages)(uint64_t, efi_uint_t);
  efi_status_t (*get_memory_map)(efi_uint_t *, struct efi_memory_descriptor *,
                                 efi_uint_t *, efi_uint_t *, uint32_t *);
  efi_status_t (*allocate_pool)(enum efi_memory_type, efi_uint_t, void **);
  efi_status_t (*free_pool)(void *);

  // Event & Timer Services
  void (*unused7)(void);
  void (*unused8)(void);
  void (*unused9)(void);
  void (*unused10)(void);
  void (*unused11)(void);
  void (*unused12)(void);

  // Protocol Handler Services
  void (*unused13)(void);
  void (*unused14)(void);
  void (*unused15)(void);
  void (*unused16)(void);
  void *reserved;
  void (*unused17)(void);
  void (*unused18)(void);
  void (*unused19)(void);
  void (*unused20)(void);

  // Image Services
  void (*unused21)(void);
  void (*unused22)(void);
  void (*unused23)(void);
  void (*unused24)(void);

  efi_status_t (*exit_boot_services)(efi_handle_t, efi_uint_t);

  // Miscellaneius Services
  void (*unused26)(void);
  void (*unused27)(void);
  void (*unused28)(void);

  // DriverSupport Services
  void (*unused29)(void);
  void (*unused30)(void);

  // Open and Close Protocol Services
  efi_status_t (*open_protocol)(efi_handle_t, struct efi_guid *, void **,
                                efi_handle_t, efi_handle_t, uint32_t);
  efi_status_t (*close_protocol)(efi_handle_t, struct efi_guid *, efi_handle_t,
                                 efi_handle_t);
  void (*unused33)(void);

  // Library Services
  efi_status_t (*protocols_per_handle)(efi_handle_t, struct efi_guid ***,
                                       efi_uint_t *);
  void (*unused35)(void);
  efi_status_t (*locate_protocol)(struct efi_guid *, void *, void **);
  // void (*unused36)(void);
  void (*unused37)(void);
  void (*unused38)(void);

  // 32-bit CRC Services
  void (*unused39)(void);

  // Miscellaneius Services (cont)
  void (*unused40)(void);
  void (*unused41)(void);
  void (*unused42)(void);
};

#endif  // __EFI_BOOT_TABLE_H__
