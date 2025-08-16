#include "acpi/acpi.h"
#include "acpi/pic.h"
#include "acpi/pic.h"
#include "types.h"
#include "ioapic.h"
#include "lapic.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MADT_ADDR32(index) madt->interrupt_controller_structure[index + 3] << 24 | madt->interrupt_controller_structure[index + 2] << 16 | madt->interrupt_controller_structure[index + 1] << 8 | madt->interrupt_controller_structure[index];

bool sdt_checksum(acpi_sdt_header_t* sdt)
{
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t sum    = 0;
  for (int i = 0; i < sdt->length; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

static inline void io_wait(void)
{
  outb(0x80, 0);
}

void start_pic()
{
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, 0x20);
  io_wait();
  outb(PIC2_DATA, 0x28);
  io_wait();
  outb(PIC1_DATA, 1 << CASCADE_IRQ);
  io_wait();
  outb(PIC2_DATA, 2);
  io_wait();
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(PIC1_DATA, 0);
  outb(PIC2_DATA, 0);
}

void mask_pic()
{
  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);
}

void parse_madt(uint64_t address)
{
  acpi_madt_t* madt       = (void*)address;
  uint64_t length_in_byte = madt->header.length - 44;
  printf("MADT\n\tSignature: %.4s\n\tLength: %d\n\tPIC Compat: %d\n", madt->header.signature, length_in_byte, madt->flags & 0b1);
  if ((madt->flags & 0b1) == 1)
  {
    mask_pic();
  }
  int i = 0;

  while (i < length_in_byte)
  {
    uint8_t type   = (uint8_t)madt->interrupt_controller_structure[i];
    uint8_t length = (uint8_t)madt->interrupt_controller_structure[i + 1];
    switch (type)
    {
      case 1:
        {
          uint32_t addr = MADT_ADDR32(i + 4); // INCORRECT
          uint32_t gsi  = MADT_ADDR32(i + 8);
          program_ioapic(0xFEC00000, gsi);
          break;
        }
      default:
        {
        }
    }
    i += length;
  }
  lapic_enable(madt->local_interrupt_controller_address);
}

void setup_acpi(uint64_t xsdt_address)
{
  acpi_xsdt_t* description_table = (void*)VIRTUAL(xsdt_address);
  if (!sdt_checksum(&description_table->header))
  {
    printf("Failed to parse ACPI tables\n");
  }

  uint64_t entry_count = (description_table->header.length - 36) / 8;

  for (int i = 0; i < entry_count; i++)
  {
    acpi_sdt_header_t* header = (void*)VIRTUAL(description_table->entry[i]);
    if (!sdt_checksum(header))
    {
      printf("Invalid ACPI Table\n");
      return;
    }

    if (strncmp(header->signature, "APIC", 4) == 0)
    {
      parse_madt(description_table->entry[i]);
    }
  }
  printf("Finished parsing ACPI tables\n");
}
