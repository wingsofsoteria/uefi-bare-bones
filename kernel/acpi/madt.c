#include "madt.h"
#include "acpi/pic.h"
#include "stdio.h"
#include "stdlib.h"
#include "lapic.h"
#define MADT_ADDR32(index) (madt->interrupt_controller_structure[index + 3] & 0xFF) << 24 | (madt->interrupt_controller_structure[index + 2] & 0xFF) << 16 | (madt->interrupt_controller_structure[index + 1] & 0xFF) << 8 | madt->interrupt_controller_structure[index] & 0xFF;
#define MADT_LOOP          for (uint64_t i = 0; i < madt->header.length - 44; i += madt->interrupt_controller_structure[i + 1])
// TODO better helper functions (for each of the interrupt controller structures, etc)
// TODO better data structures
// TODO properly support old PIC

static acpi_madt_t* madt = NULL;
void parse_madt();
void set_madt(uint64_t address)
{
  madt = (void*)address;
  parse_madt();
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

void parse_madt()
{
  set_lapic_addr(madt->local_interrupt_controller_address);
  printf("MADT\n\tSignature: %.4s\n\tLength: %d\n\tPIC Compat: %d\n", madt->header.signature, madt->header.length - 44, madt->flags & 0b1);
  if ((madt->flags & 0b1) == 1)
  {
    mask_pic();
  }
}

madt_interrupt_source_override_t* madt_get_override_for_irq(uint8_t irq)
{
  MADT_LOOP
  {
    uint8_t type = (uint8_t)madt->interrupt_controller_structure[i];
    if (type != 2) continue;
    uint8_t current_irq = madt->interrupt_controller_structure[i + 3];
    if (current_irq != irq) continue;
    return (void*)(madt->interrupt_controller_structure + i);
  }
  return NULL;
}

uint32_t madt_get_ioapic(uint32_t gsi)
{
  MADT_LOOP
  {
    uint8_t type = (uint8_t)madt->interrupt_controller_structure[i];
    if (type != 1) continue;
    uint32_t current_gsi = MADT_ADDR32(i + 8);
    if (current_gsi != gsi) continue;
    return MADT_ADDR32(i + 4);
  }

  printf("No IO APIC with GSI base %d\n", gsi);
  abort();
}
