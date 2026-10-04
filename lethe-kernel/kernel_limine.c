#include "acpi/acpi.h"
#include "memory/alloc.h"
#include "stdbool.h"
#include "stddef.h"
#include "terminal/pixel.h"
#include "utils.h"

#include <stdint.h>
#ifdef KERNEL_USE_LIMINE
#define __BOOTLOADER_SPECIFIC_CODE
#include "loaders/limine.h"
#define LIMINE_REQUESTS_START_MARKER \
  { 0xF6B8F4B39DE7D1AE,              \
    0xFAB91A6940FCB9CF,              \
    0x785C6ED015D3E316,              \
    0x181E920A7852B9D9 }
static __attribute__((
  used,
  section(".limine_requests")
)) volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_framebuffer_request framebuffer_request = {
  .id       = LIMINE_FRAMEBUFFER_REQUEST_ID,
  .revision = 0,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_memmap_request memmap_request = {
  .id       = LIMINE_MEMMAP_REQUEST_ID,
  .revision = 0,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_hhdm_request hhdm_request = {
  .id       = LIMINE_HHDM_REQUEST_ID,
  .revision = 0,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_paging_mode_request paging_mode_request = {
  .id       = LIMINE_PAGING_MODE_REQUEST_ID,
  .revision = 1,
  .mode     = LIMINE_PAGING_MODE_X86_64_4LVL,
  .max_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
  .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_rsdp_request rsdp_request = {
  .id       = LIMINE_RSDP_REQUEST_ID,
  .revision = 0,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_executable_file_request executable_request = {
  .id       = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
  .revision = 0,
};

static __attribute__((
  used,
  section(".limine_requests")
)) volatile struct limine_tsc_frequency_request tsc_frequency = {
  .id       = LIMINE_TSC_FREQUENCY_REQUEST_ID,
  .revision = 0,
};

#define LIMINE_REQUESTS_END_MARKER { 0xADC0E0531BB10D03, 0x9572709F31764C62 }

uint64_t hhdm_mapping = 0;
// NOLINTNEXTLINE
void*    kernel_file_address;

static void feature_check()
{
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) halt();

  if (
    framebuffer_request.response == NULL ||
    framebuffer_request.response->framebuffer_count < 1
  )
    halt();

  if (memmap_request.response == NULL) halt();

  hhdm_mapping = 0;
  if (hhdm_request.response != NULL)
    {
      struct limine_hhdm_response* hhdm_response = hhdm_request.response;
      hhdm_mapping                               = hhdm_response->offset;
    }

  struct limine_framebuffer* framebuffer =
    framebuffer_request.response->framebuffers[0];
  if (framebuffer->bpp != 32) halt();

  if (rsdp_request.response == NULL) halt();

  if (executable_request.response == NULL) halt();

  if (tsc_frequency.response != NULL)
    kernel_config.timers.tsc_freq_khz =
      tsc_frequency.response->frequency * 1000;
  kernel_file_address = executable_request.response->executable_file->address;
}

// NOLINTNEXTLINE
void bootloader_specific_init()
{
  feature_check();
  init_fb(framebuffer_request.response);
  setup_allocator(memmap_request.response);
  acpi_early_init(rsdp_request.response->address);
}

#endif
